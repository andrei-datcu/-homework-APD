/*
 * Autor: Datcu Andrei Daniel
 * Grupa: 331CC
 * Data: 14.12.2013
 *
 * Tema 3 APD
 *
 * main.cpp
 *
 * Fisierul care implementeaza cele 2 tipuri de comportamente ale proceselor
 * (master si worker).
 *
 * Masterul citeste din fisier, trimite datele workerilor si isi calculeaza
 * bucata sa. Workerii isi calculeaza bucatile lor pe care le trimit masterului
 * Acesta le uneste si salveaza fisierul.
 *
 */

#include <mpi.h>
#include <fstream>
#include <complex>
#include <cstring>
#include <cmath>

#include "matrix.h"

#define NUM_COLORS 256

void calc_mandelbrot(const std::complex<double> &zmin, const double resolution,
        const int MAX_STEPS, Matrix<unsigned char> &image){

    /*
     * Functie care calculeaza multime tip Mandelbrot.
     *
     * Parametrii input:
     *
     * zmin = punctul de unde incepe planul
     * resolution = pasul de discretizare al planului cimplex
     * MAX_STEPS = ca in enunt
     * image = matricea imagine unde vor fi stocate rezultatele
     *
     */

    for (int y = 0; y < image.lineCount; ++y)
        for (int x = 0; x < image.colCount; ++x){

            std::complex<double> c(x * resolution, y * resolution);
            c += zmin;
            std::complex<double> z(0, 0);
            int step = 0;
            while (std::abs(z) < 2 && step < MAX_STEPS){

                z = z * z + c;
                ++step;
            }

            image[y][x] = step % NUM_COLORS;
        }
}

void calc_julia(const std::complex<double> &zmin, const std::complex<double>
        &c, const double resolution, const int MAX_STEPS,
        Matrix<unsigned char> &image){

    /*
     * Functie care calculeaza multime tip Julia.
     *
     * Parametrii input:
     *
     * zmin = punctul de unde incepe planul
     * c = parametrul specific functiei rationale julia
     * resolution = pasul de discretizare al planului cimplex
     * MAX_STEPS = ca in enunt
     * image = matricea imagine unde vor fi stocate rezultatele
     *
     */

    for (int y = 0; y < image.lineCount; ++y)
        for (int x = 0; x < image.colCount; ++x){

            std::complex<double> z(x * resolution, y * resolution);
            z += zmin;
            int step = 0;
            while (std::abs(z) < 2 && step < MAX_STEPS){

                z = z * z + c;
                ++step;
            }

            image[y][x] = step % NUM_COLORS;
        }
}

int main(int argc, char **argv){


    int rank, nproc;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    std::complex<double> zmin, zmax, zjulia;
    double resolution;
    bool julia;
    int MAX_STEPS;

    if (rank == 0){//Daca este procesul master

        //Citesc datele din fisier

        std::ifstream fin(argv[1], std::ios::in);
        fin >> julia;
        double real, real2, im, im2;
        fin >> real >> real2 >>  im >> im2;
        zmin = std::complex<double>(real, im);
        zmax = std::complex<double>(real2, im2);
        fin >> resolution >> MAX_STEPS;

        if (julia){//Daca voi calcula multime julia, citesc parametrul extra

            fin >> real2 >> im2;
            zjulia = std::complex<double>(real2, im2);
        }

        fin.close();

        //transmit datele citite tuturor proceselor

        for (int i = 1; i < nproc; ++i){
            MPI_Send(&julia, 1, MPI_CHAR, i, 1, MPI_COMM_WORLD);

            real = zmin.real();
            im = zmin.imag();
            MPI_Send(&real, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
            MPI_Send(&im, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);

            real = zmax.real();
            im = zmax.imag();
            MPI_Send(&real, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
            MPI_Send(&im, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);

            MPI_Send(&resolution, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
            MPI_Send(&MAX_STEPS, 1, MPI_INT, i, 1, MPI_COMM_WORLD);

            if (julia){
                real = zjulia.real();
                im = zjulia.imag();
                MPI_Send(&real, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                MPI_Send(&im, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
            }
        }
    }
    else {// Daca procesul este worker

        //Primesc datele problemei de la master

        double real, im;
        MPI_Recv(&julia, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);

        MPI_Recv(&real, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&im, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        zmin = std::complex<double>(real, im);

        MPI_Recv(&real, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&im, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        zmax = std::complex<double>(real, im);

        MPI_Recv(&resolution, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&MAX_STEPS, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

        if (julia) {
            MPI_Recv(&real, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&im, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
            zjulia = std::complex<double>(real, im);
        }
    }

    //Calculez dimensiunile imaginei finale

    int fullWidth = floor((zmax.real() - zmin.real()) / resolution);
    int fullHeight = floor((zmax.imag() - zmin.imag()) / resolution);

    int linechunk = ceil(fullHeight / nproc); //grosimea fasiei locale

    //punctul de start al planului local
    std::complex<double> local_zmin(zmin.real(),
            zmin.imag() + linechunk * rank * resolution);


    if (rank == nproc - 1) //daca sunt ultimul proces, merg pana la capat
        linechunk = fullHeight - linechunk * (nproc - 1);

    Matrix<unsigned char> local_image(linechunk, fullWidth);

    if (julia)
        calc_julia(local_zmin, zjulia, resolution, MAX_STEPS, local_image);
    else
        calc_mandelbrot(local_zmin, resolution, MAX_STEPS, local_image);

    if (rank > 0){// Daca procesul este worker

        //trimite masterului rezultatele locale

        MPI_Send(local_image.getData(),
                local_image.lineCount * local_image.colCount, MPI_CHAR, 0, 1,
                MPI_COMM_WORLD);
    }
    else{//Daca procesul este master

        //Construiesc imaginea finala
        Matrix <unsigned char> finalImage(fullHeight, fullWidth);

        //Mai intai copiez rezultatele mele locale in imaginea finala
        for (int i = 0; i < linechunk; ++i)
            memcpy(finalImage[i], local_image[i],
                    sizeof(unsigned char) * fullWidth);

        //Calculez si latimea fasiei ultimului worker
        int last_height = fullHeight - linechunk * (nproc - 1);

        //Primesc imaginile locale de la ceilalti workeri

        Matrix<unsigned char> recv_image_n(linechunk, fullWidth);
        Matrix<unsigned char> recv_image_last(last_height, fullWidth);

        for (int i = 1; i < nproc; ++i){

            Matrix<unsigned char> &recv_image = i == nproc - 1 ?
                recv_image_last : recv_image_n;

            MPI_Recv(recv_image.getData(),
                    recv_image.lineCount * recv_image.colCount,
                    MPI_CHAR, i, 1, MPI_COMM_WORLD, &status);

            //Asez fiecare linie din matricea primita in locaul corespunzator
            //in imaginea finala
            for (int j = 0; j < recv_image.lineCount; ++j)
                memcpy(finalImage[linechunk * i + j], recv_image[j],
                        sizeof(unsigned char) * recv_image.colCount);
        }

        //Scriu imaginea finala in fisier tip pgm

        std::ofstream fout(argv[2], std::ios::out);

        fout << "P2" << std::endl;
        fout << finalImage.colCount << " " << finalImage.lineCount<< std::endl;
        fout << NUM_COLORS - 1 << std::endl;

        for (int line = finalImage.lineCount - 1; line >= 0; --line){
            for (int col = 0; col < finalImage.colCount; ++col)
                fout << (unsigned int)finalImage[line][col] << " ";
            fout << std::endl;
        }
        fout.close();
    }

    MPI_Finalize();
    return 0;
}
