/*
 * Datcu Andrei Daniel 331CC
 * Tema1 APD
 * Rezolvarea problemei paralel, neoptimizat
 * Timp de executie O(s * n ^ 4)
 *
 * 08.11.2013
 *
 * NOTA: Aceasta sursa nu contine comentarii, fiind foarte asemanatoare
 * cu varianta sa paralela. Pentru mai multe comentarii vezi README
 * si serialneoptim.cpp
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <bitset>

#define NCMAX 50
#define NMAX 100
#define MAXDISTANCE (NMAX + 1)

#define CHECK_NEIGHBOUR \
                    if (line >= 0 && line <= (n - 1) &&\
                            col >= 0 && col<= (n - 1)){\
                        vecinColor = colors[line][col];\
                        if (distances[vecinColor] > radius){\
                            colorsFound.set(vecinColor);\
                            distances[vecinColor] = radius;\
                        }\
                    }

typedef int ColorMatrix[NMAX][NMAX];
typedef int SizeArray[NCMAX];
typedef std::pair<int, int> senator;


void simweek(int nc, int n, ColorMatrix &colors, SizeArray &sizes){

    static ColorMatrix temp;
    static unsigned int noOfDifferentColors = nc;

    memset(sizes, 0, NCMAX * sizeof(int));

    std::bitset<NCMAX> colorsPresentInNextMatrix;

    #pragma omp parallel for schedule(dynamic)\
        shared(n, nc, colors, sizes, temp, colorsPresentInNextMatrix)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j){

            char distances[NCMAX];

            memset(distances, MAXDISTANCE, NCMAX * sizeof(char));
            std::bitset<NCMAX> colorsFound;

            int vecinColor, nextColor;
            char max = 0;

            const char
                maxRadius = std::max(std::max(i, n - i), std::max(j, n - j));

            int line, col;

            for(char radius = 1; radius <= maxRadius &&
                    colorsFound.count() < noOfDifferentColors; ++radius){

                line = i - radius;
                for (col = j - radius; col <= j + radius; ++col)
                    CHECK_NEIGHBOUR;

                line = i + radius;
                for (col = j - radius; col <= j + radius; ++col)
                    CHECK_NEIGHBOUR;

                col = j - radius;
                for (line = i - radius; line <= i + radius; ++line)
                    CHECK_NEIGHBOUR;

                col = j + radius;
                for (line = i - radius; line <= i + radius; ++line)
                    CHECK_NEIGHBOUR;
            }

            for (int k = 0; k < nc; ++k)
                if (max < distances[k] && distances[k] != MAXDISTANCE){
                    nextColor = k;
                    max = distances[k];
                }

            temp[i][j] = nextColor;

            #pragma omp critical
            {
            ++sizes[nextColor];
            colorsPresentInNextMatrix.set(nextColor);
            }
        }

    noOfDifferentColors = colorsPresentInNextMatrix.count();
    memcpy(colors, temp, NMAX * NMAX * sizeof(int));
}

int main(int argc, char **argv){

    if (argc != 4){

        std::cerr << "Wrong paramaters. Usage " <<argv[0] <<
            " nr_saptamani fisin fisout" << std::endl;
        return -1;
    }

    int s = atoi(argv[1]);

    std::ifstream fin(argv[2], std::ifstream::in);

    int n, nc;
    fin >> n >> nc;

    ColorMatrix currentMatrix;
    SizeArray sizes;


    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            fin >> currentMatrix[i][j];

    fin.close();

    std::ofstream fout(argv[3]);

    for (int w = 0; w < s; ++w){
        simweek(nc, n, currentMatrix, sizes);

       for (int j = 0; j < nc; ++j)
           fout << sizes[j] << " ";

       fout << std::endl;
    }


    for (int i = 0; i < n; ++ i){
        for (int j = 0; j < n; ++ j)
            fout << currentMatrix[i][j] << " ";

        fout << std::endl;
    }

    fout.close();


    return 0;
}
