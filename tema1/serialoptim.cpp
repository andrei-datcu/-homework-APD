/*
 * Datcu Andrei Daniel 331CC
 * Tema1 APD
 * Rezolvarea problemei serial, optimizat
 * Timp de executie O(s * n ^ 3)
 *
 * 08.11.2013
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <list>
#include <queue>

#define NCMAX 50
#define NMAX 100
#define MAXDISTANCE (NMAX + 1)

typedef std::pair<int, int> senator;
//un senator este definit de pozitia sa in sala


typedef std::list<senator> ColorList[NCMAX];
/*
 * sala va fi memorata ca un vector de liste
 *
 * Fiecare culoare va avea o lista ce va contine
 * senatorii de aceasta culoare
*/


void simweek(int nc, int n, ColorList &colors){

    /*
     * Functie care simuleaza o saptamana
     *
     * Parametrii:
     *  nc = numar culor (ca in enunt)
     *  n = numar senator (ca in enunt)
     *  colors = listele culorilor (Vezi explicatia de la typedef)
    */

    static const int dc[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    static const int dl[8] = {1, 1, 0, -1, -1, -1, 0, 1};


    static char situation[NCMAX][NMAX][NMAX];

    /*
     * Matricea de distante
     *
     * situation[k][i][j] = distanta minima de la senatorul i j
     *  la un senator k diferit de el (conform enuntului)
     *
     *  La inceputul oricarei saptamani este plina de 0
     *
    */

    for (int i = 0; i < nc; ++i)
        if (!colors[i].empty()){// culoarea inca mai exista pe scena politica

            senator curent, vecin;
            std::queue<senator> searchQueue; //coada bf
            static senator father[NMAX][NMAX];
            //mat de tati spune de unde a plecat zona in care s(i,j) se afla

            char twsituation[NMAX][NMAX] = {{0}};
            //distantele minime intre senatorii de culoarea i (vezi README)

            //adaug initial in coada toti senatorii de culoarea i, fiecare zona
            for (senator &s : colors[i]){
                searchQueue.push(s);
                twsituation[s.first][s.second] = MAXDISTANCE;
                father[s.first][s.second] = s;
                //zona sa pleaca chiar de la el
            }


            while (!searchQueue.empty()){
                curent = searchQueue.front();

                for (int k = 0; k < 8; ++k){//pentru fiecare directie
                    vecin = std::make_pair(curent.first + dl[k],
                            curent.second + dc[k]);

                    //verific daca raman in limitele matricei
                    if (vecin.first >= 0 && vecin.first <= (n - 1) &&
                            vecin.second >= 0 && vecin.second <= (n - 1))

                        //daca da verific sa nu intru peste alta zona sau
                        //peste tatal meu
                        if (situation[i][vecin.first][vecin.second] == 0  &&
                                twsituation[vecin.first][vecin.second] == 0){

                            searchQueue.push(vecin);
                            situation[i][vecin.first][vecin.second] =
                              situation[i][curent.first][curent.second]  + 1;
                            father[vecin.first][vecin.second] =
                                father[curent.first][curent.second];
                        }
                        else{// s-au intalnit doua zone

                            senator f_vecin =father[vecin.first][vecin.second],
                                f_curent = father[curent.first][curent.second];
                            if (f_curent != f_vecin){//daca zonele sunt diferite

                                //calculez distanta intre tatii lor
                                int distance = situation[i][vecin.first]
                                    [vecin.second] + situation[i]
                                    [curent.first][curent.second] + 1;

                                //verific daca este distanta minima pentru
                                //vreunul din cei doi

                                if (twsituation[f_vecin.first][f_vecin.second]
                                        > distance)
                                    twsituation[f_vecin.first][f_vecin.second]
                                        = distance;

                                if(twsituation[f_curent.first][f_curent.second]
                                        > distance)
                                    twsituation[f_curent.first]
                                        [f_curent.second] = distance;
                            }
                        }
                }
                searchQueue.pop();
            }

            //pun situatiile "speciale" la locul lor
            //daca twsituation e nemodificat inseamna
            //ca un senator e singur in partid

            for (senator s: colors[i])
                situation[i][s.first][s.second] =
                    twsituation[s.first][s.second] == MAXDISTANCE ? 0 :
                     twsituation[s.first][s.second];

        }

    //noile liste, temporare
    ColorList temp;

    for (int i = 0; i < nc; ++i)
        for (senator s : colors[i]){

            int max = 0, nextColor = i;
            for (int j = 0; j < nc; ++j){
                if (max < situation[j][s.first][s.second]){
                    max = situation[j][s.first][s.second];
                    nextColor = j;
                }
                situation[j][s.first][s.second] = 0;
            }
            temp[nextColor].push_back(s);
        }

    for (int i = 0; i < nc; ++i)
        colors[i].swap(temp[i]);

}

int main(int argc, char **argv){

    if (argc != 4){

        std::cerr << "Wrong paramaters. Usage " <<argv[0] <<
            " nr_saptamani fisin fisout" << std::endl;
        return -1;
    }

    int s = atoi(argv[1]);

    std::ifstream fin(argv[2], std::ifstream::in);

    int n, nc, c;
    fin >> n >> nc;

    ColorList colors;

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j){
            fin >> c;
            colors[c].push_back(std::make_pair(i, j));
        }

    fin.close();

    int matrix[NMAX][NMAX];

    std::ofstream fout(argv[3]);

    for (int w = 0; w < s; ++w){
        simweek(nc, n, colors);

       for (int j = 0; j < nc; ++j)
           fout << colors[j].size() << " ";

        fout << std::endl;
    }


    for (int i = 0; i < nc; ++i)
        for (senator sen : colors[i])
            matrix[sen.first][sen.second] = i;

    for (int i = 0; i < n; ++ i){
        for (int j = 0; j < n; ++ j)
            fout << matrix[i][j] << " ";

        fout << std::endl;
    }

    fout.close();

    return 0;
}
