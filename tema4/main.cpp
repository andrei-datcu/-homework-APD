/*
 * Autor: Datcu Andrei Daniel
 * Grupa: 331CC
 * Data: 23.12.2013
 *
 * Tema 4 APD
 *
 * main.cpp
 *
 * Fisierul care implementeaza cele 3 faze ale enuntului temei.
 *
 * 0) Citire din fisier
 * 1) Stabilirea topologiei tip arbore
 * 2) Trimiterea si rutarea mesajelor
 * 3) Stabilirea unui presedinte si a unui vicepresedinte
 *
 */

#include <mpi.h>

#include "matrix.h"
#include "message.h"
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <cstdlib>

#define DEBUG_MODE 0
#define debug if (DEBUG_MODE) std::cerr

void readTopology(int rank, int nproc, char *nume_fisier,
        Matrix<unsigned char> &topologie){

/*
 * Functie care citeste linia corespunzatoare rankului
 * din fisierul trimis ca primul argument programului
 *
 * Argumente de intrare:
 *   rank - id-ul procesului curent
 *   nproc - numarul total de procese
 *   nume_fisier - numele fisierului care contine topolgoia
 *
 * Argumente de iesire
 *  topolgoie - matricea de adiacenta care contine doar muchiile
 *  care au treaba cu procesul curent
 *
 */

    std::ifstream fin(nume_fisier);
    std::string tmp;

    for (int lc = 0; lc < rank + 1 ; ++lc)
        std::getline(fin, tmp);


    std::stringstream line(tmp);

    line.ignore(4);
    int v;
    while (line >> v)
        topologie[rank][v] = topologie[v][rank] = 1;

    fin.close();
}

int getFinalTopology(int rank, int nproc, Matrix<unsigned char> &topologie){

/*
 * Functie care stabileste topologia finala - arborele de acoperire
 *
 * Argumente de intrare:
 *   rank - id-ul procesului curent
 *   nproc - numarul de procese
 *   topologie - topologia initiala (doar cu muchiile ce tin de procesul rank)
 *
 * Iesire:
 *  topologia va fi modificata a.i sa contina tot arborele
 */

    char ecou = 'e', sondaj = 's';
    MPI_Status status;

    int no_send = 0;

    if (rank == 0){

        for (int i = 1; i < nproc; ++i)
            if (topologie[rank][i]){
                MPI_Send(&sondaj, 1, MPI_CHAR, i, 1, MPI_COMM_WORLD);
                no_send++;
            }
    }

    int parent = -1;
    bool ecou_recv = false;

    std::vector<bool> sons(nproc, true);
    sons[rank] = false;

    do{

        char mesaj;
        MPI_Recv(&mesaj, 1, MPI_CHAR, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD,
                &status);
        int source = status.MPI_SOURCE;

        if (mesaj == sondaj){

            debug << "Node " << rank << " am primit sondaj de la " << source
                << std::endl;

            sons[source] = false;
            // le voi folosi pentru a trimtietopologia completa inapoi

            if (parent == -1){
                parent = status.MPI_SOURCE;

                for(int i = 0; i < nproc; ++i)
                    if (topologie[rank][i] == 1 && i != parent){
                        ++no_send;
                        MPI_Send(&sondaj, 1, MPI_CHAR, i, 1, MPI_COMM_WORLD);
                    }

                for (int i = 0; i < nproc; ++i)
                    topologie[rank][i] = topologie[i][rank] = 0;

                topologie[rank][parent] = topologie[parent][rank] = 1;
            }
            else
                --no_send;
        }
        else{ // este ecou
            debug << "Node " << rank << " am primit ecou de la " << source
                << std::endl;
            static Matrix<unsigned char> recv_top(nproc, nproc);

            MPI_Recv(recv_top.getData(), nproc * nproc, MPI_CHAR,
                    source, 2, MPI_COMM_WORLD, &status);

            topologie |= recv_top;
            --no_send;
        }
    } while (no_send != 0);

    debug << "Node " << rank << " am iesit din bucla" << std::endl;


    if (parent != -1){ // aka rank == 0

        MPI_Send(&ecou, 1, MPI_CHAR, parent, 1, MPI_COMM_WORLD);

        MPI_Send(topologie.getData(), nproc * nproc, MPI_CHAR, parent, 2,
                MPI_COMM_WORLD);

        MPI_Recv(topologie.getData(), nproc * nproc, MPI_CHAR, parent, 2,
                MPI_COMM_WORLD, &status);
    }

    for (int i = 0; i < nproc; ++i)
        if (sons[i])
            MPI_Send(topologie.getData(), nproc * nproc, MPI_CHAR, i, 2,
                MPI_COMM_WORLD);
    return parent;
}

std::vector<int> calculateRoutes(Matrix<unsigned char> &top, int rank,
        int nproc){

/*
 * Functie care calculeaza tabela de rutare folosind o parcurgere bf
 *
 * Argumente de intrare:
 *  top - matricea ce contine topolgia
 *  rank - id-ul procesului curent
 *  nproc - numarul de procese
 *
 * Intoarce: Un vector de nproc elemente - tabela de rutare
 */

    std::vector<int> result(nproc, -1);
    std::queue<int> bf_queue;
    std::vector<bool> visited(nproc, false);

    bf_queue.push(rank);
    visited[rank] = true;
    result[rank] = rank;

    while (!bf_queue.empty()){

        int cnode = bf_queue.front();

        for (int i = 0; i < nproc; ++i)
            if (top[cnode][i] == 1 && !visited[i]){

                bf_queue.push(i);
                visited[i] = true;
                result[i] = cnode == rank ? i : result[cnode];
            }
        bf_queue.pop();
    }

    return result;
}

void printMatrix(Matrix<unsigned char> &mat){

/*
 * Functie pentru afisarea unei matrici topologie la stdout
*/

    for (int i = 0; i < mat.lineCount; ++i){
        for (int j = 0; j < mat.colCount; ++j)
            std::cout << (int)mat[i][j] << " ";

        std::cout << '\n';
    }
}

void printTable(const std::vector<int> table){

/*
 * Functie pentru afisarea unei tabele de rutare la stdout
*/

    std::cout << "Destination       Next HOP\n";
    std::cout << "--------------------------\n";

    for (int i = 0; i < table.size(); ++i)
        std::cout << i << "                  " << table[i] << "\n";

    std::cout << "--------------------------\n" << std::endl;
}

void readAndSendMyMessages(int rank, char *filePath,
        const std::vector<int> &table, Matrix<unsigned char> &topology){

/*
 * Functie care citeste liniile cu mesaje ce au ca sursa procesul curent
 * initiaza comunicarea, trimite mesajele si incheie comunicarea
 *
 * Argumente de intrare:
 *   rank - id-ul procesului curent
 *   filePath - numele fisierului care contine mesajele
 *   table - tabela de rutare
 *   topology - matricea de adiacenta
 *
 */

    std::ifstream fin(filePath, std::ios::in);

    int n;
    fin >> n;

    Message startmsg(rank, 255, 1, "Start");

    for (int i = 0; i < topology.lineCount; ++i)
        if (topology[rank][i])
            startmsg.sendTo(i);

    debug << "Node " << rank << "citesc din fisier" << std::endl;

    for (int i = 0; i < n; ++i){
        int source;
        fin >> source;

        std::string tempstr;
        std::getline(fin, tempstr);

        if (rank != source)
            continue;

        std::stringstream ss(tempstr);
        ss.ignore(1);
        ss >> tempstr;
        ss.ignore(1);

        int dest = tempstr[0] == 'B' ? 255 : atoi(tempstr.c_str());

        std::getline(ss, tempstr);

        Message msg(source, dest, 0, tempstr);

        if (!msg.isBroadcast())
            msg.sendTo(table[dest]);
        else
            for (int i = 0; i < topology.lineCount; ++i)
                if (topology[rank][i])
                    msg.sendTo(i);
    }

    fin.close();

    Message endmsg(rank, 255, 2, "End");

    for (int i = 0; i < topology.lineCount; ++i)
        if (topology[rank][i])
            endmsg.sendTo(i);

    debug << "Node " << rank << "am citit din fisier" << std::endl;
}

void routeMessages(int rank, int nproc, const std::vector<int> &routeTable,
                   Matrix<unsigned char> &topology){

/*
 * Functie care ruteaza mesajele
 *
*/

    int nodesStillSending = nproc - 1; //toti in afara de mine

    Message msg;

    while (nodesStillSending){

        msg.receive();

        if (msg.destination() == rank || msg.isBroadcast()){
            //sunt destinatarul mesajului

            std::cout << "[Node " << rank << "]Am primit mesajul de la "
                << msg.source() << " cu textul: " << msg.text() << std::endl;
        }
        if (msg.destination() != rank && !msg.isBroadcast()){

            std::cout << "[Node " << rank << "]Am primit mesaj cu sursa "
                << msg.source() << " cu destinatia " << msg.destination()
                << "il redirectionez catre " << routeTable[msg.destination()]
                << std::endl;

            msg.sendTo(routeTable[msg.destination()]);
        }

        if (msg.isBroadcast()){

            std::cout << "[Node " << rank << "] redirectionez mesaj broadcast"
                << std::endl;

            for (int i = 0; i < nproc; ++i)
                if (topology[i][rank] == 1 && i != msg.recv_source)
                    msg.sendTo(i);
        }

        if (msg.lastMessage())
            nodesStillSending--;
    }
}

int vote(int nproc, int except = -1){

    int result;

    do
        result = rand() % nproc;
    while (result == except);
    return result;
}

void votePresidents(Matrix<unsigned char> &top, int parent, int rank,
                    int nproc){

/*
 * Functie care voteaza pentru presedinte si vicepresedinte,
 * centralizeaza rezultatele de la fii si le trimite sub
 * forma de vector la parinte, apoi asteapta broadcastul
 * cu rezultatele finale, dupa care le afiseaza
 *
 * Argumente de intrare:
 *   top - matricea de adiacenta
 *   parent - idul parintelui
 *   rank - idul procesului curent
 *   nproc - numarul de procese
 *
 */

    srand((int)time(NULL) + 100000000 * rank);

    int president = vote(nproc);
    int vpresident = vote(nproc, president);

    int *votes = new int[nproc](), *tempvotes = new int[nproc];
    MPI_Status status;

    votes[president] = votes[vpresident] = 1;

    debug << "Nodul " << rank << " a votat " <<president << " si " <<vpresident
        << std::endl;

    for (int i = 0; i < nproc; ++i)
        if (i != rank && top[i][rank] && i != parent){

            MPI_Recv(tempvotes, nproc, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
            for (int j = 0; j < nproc; ++j)
                votes[j] += tempvotes[j];
        }

    debug <<"Nodul " << rank << "trimit rezultate la parinte" << std::endl;
    if (parent != -1)
        MPI_Send(votes, nproc, MPI_INT, parent, 1, MPI_COMM_WORLD);

    if (rank == 0){

        int pi, vpi; //index president si vp

        if (votes[0] > votes[1]){
            pi = 0;
            vpi = 1;
        }
        else{
            pi = 1;
            vpi = 0;
        }

        for (int i = 2; i < nproc; ++i)
            if (votes[i] > votes[pi]){
                vpi = pi;
                pi = i;
            }
            else
                if (votes[i] > votes[vpi])
                    vpi = i;

        president = pi;
        vpresident = vpi;
    }

    if (parent != -1){
        MPI_Recv(&president, 1, MPI_INT, parent, 2, MPI_COMM_WORLD, &status);
        MPI_Recv(&vpresident, 1, MPI_INT, parent, 2, MPI_COMM_WORLD, &status);
    }

    for (int i = 0; i < nproc; ++i)
        if (i != rank && top[i][rank] && i != parent){
            MPI_Send(&president, 1, MPI_INT, i, 2, MPI_COMM_WORLD);
            MPI_Send(&vpresident, 1, MPI_INT, i, 2, MPI_COMM_WORLD);
        }

    std::cout << "[Node " << rank << "]" << "presedinte: " << president
        << " vicepresedinte: " << vpresident << std::endl;

    delete[] votes;
    delete[] tempvotes;
}

int main(int argc, char **argv){

    int rank, nproc;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    Matrix<unsigned char> topologie(nproc, nproc);
    int parent;
    readTopology(rank, nproc, argv[1], topologie);

    //faza 1
    parent = getFinalTopology(rank, nproc, topologie);

    std::vector<int> routeTable = calculateRoutes(topologie, rank, nproc);

    std::cout << "Node number: " << rank << "\n";

    if (rank == 0)
        printMatrix(topologie);

    printTable(routeTable);

    //faza2
    readAndSendMyMessages(rank, argv[2],routeTable, topologie);
    routeMessages(rank, nproc, routeTable, topologie);
    debug << "Node " << rank << "finished routing" << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);
    std::cout.flush();

    //faza3
    votePresidents(topologie, parent, rank, nproc);

    MPI_Finalize();
    return 0;
}

