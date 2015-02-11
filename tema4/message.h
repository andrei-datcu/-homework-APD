/*
 * Autor: Datcu Andrei Daniel
 * Grupa: 331CC
 * Data: 23.12.2013
 *
 * Tema 4 APD
 *
 * message.h
 *
 * Fisierul care implementeaza clasa Message.
 *
 * Clasa message este un wrapper peste un buffer ce
 * poate fi transmis cu usurinta folosind MPI si contine
 *
 * Index sursa/destinatie (daca dest=255 atunci e broadcast)
 * Special flag (0 = mesaj normal, 1 = mesaj broadcast de start,
 *      2 = mesaj broadcast de final - conform enuntului)
 * Dimensiunea textului (incluzand ultimul octet de 0)
 * Stringul textului
 *
 */

#ifndef __MESSAGE__
#define __MESSAGE__

#include <mpi.h>
#include <cstring>
#include <string>
#include <vector>

class Message{

public:
    Message(int source, int destination, char specialbyte,
            const std::string &message){

        buffer = new char[4 + message.length() + 1];
        recv_source = -1;
        buffer[0] = source;
        buffer[1] = destination;
        buffer[2] = specialbyte;
        buffer[3] = message.length();

        strcpy(buffer + 4, message.c_str());
    }

    Message()
        : buffer(NULL), recv_source(-1) {}

    ~Message(){

        if (buffer)
            delete[] buffer;
    }

    void receive(){

        static char tempbuffer[4];
        static MPI_Status status;

        MPI_Recv(tempbuffer, 4, MPI_CHAR, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD,
                &status);

        recv_source = status.MPI_SOURCE;
        if (buffer)
            delete [] buffer;
        buffer = new char[4 + tempbuffer[3] + 1];

        memcpy(buffer, tempbuffer, 4);
        MPI_Recv(buffer + 4, tempbuffer[3] + 1, MPI_CHAR, recv_source, 2,
                MPI_COMM_WORLD, &status);
    }

    void sendTo(int destination){

        static MPI_Request req;

        MPI_Isend(buffer, 4, MPI_CHAR, destination,
                1, MPI_COMM_WORLD, &req);

        MPI_Isend(buffer + 4, buffer[3] + 1, MPI_CHAR, destination,
                2, MPI_COMM_WORLD, &req);
    }

    const int destination(){

        return buffer[1];
    }

    const int source(){

        return buffer[0];
    }

    const bool initialMessage(){

        return buffer[2] == 1;
    }

    const bool lastMessage(){

        return buffer[2] == 2;
    }

    const bool isBroadcast(){
        return (unsigned char)buffer[1] == 255;
    }

    const std::string text(){

        return std::string(buffer + 4);
    }


    int recv_source;

private:
    char *buffer;
};

#endif
