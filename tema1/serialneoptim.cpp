/*
 * Datcu Andrei Daniel 331CC
 * Tema1 APD
 * Rezolvarea problemei serial, neoptimizat
 * Timp de executie O (s * n ^ 4)
 *
 * 08.11.2013
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
// un senator e definit de pozitia sa in matrice - line, coloana


void simweek(int nc, int n, ColorMatrix &colors, SizeArray &sizes){

    /*
     * Functie care simuleaza o saptamana
     *
     * Parametrii:
     *  nc = numar culor (ca in enunt)
     *  n = numar senator (ca in enunt)
     *  colors = matricea continand culoarea fiecarui senator
     *  sizes = vector cu numarul de senatori pe care fiecare culoare ii are
    */

    static ColorMatrix temp; //matricea unde voi pune, temporar, noile culori
    static unsigned int noOfDifferentColors = nc;

    memset(sizes, 0, NCMAX * sizeof(int));

    std::bitset<NCMAX> colorsPresentInNextMatrix;

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j){

            static char distances[NCMAX];

            memset(distances, MAXDISTANCE, NCMAX * sizeof(char));

            std::bitset<NCMAX> colorsFound; //multimea culorilor gasite de acest senator

            int vecinColor, nextColor;
            char max = 0;

            const char
                maxRadius = std::max(std::max(i, n - i), std::max(j, n - j));
            //raza maxima pe care are sens sa caut o culoare

            int line, col;

            //cat timp nu am gasit toate culorile si mai am unde cauta...
            for(char radius = 1; radius <= maxRadius &&
                    colorsFound.count() < noOfDifferentColors; ++radius){

                /*
                 * Ma plimb pe cele 4 laturi ale patratului de raza radius.
                 * Desi aparent, neelegant, codul scris asa este de aproximativ 3 ori
                 * rapid decat daca as fi avut ceva de genul":
                 *      for (int pos = -radius; pos <= radius; ++pos)
                 *          in functie de pos genereaza 4 puncte si fa CHECK_NEIGHBOUR
                 *          pentru fiecare
                 *
                 * Acest lucru se intampla din cauza accesului pe matrice
                */

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
            ++sizes[nextColor];
            colorsPresentInNextMatrix.set(nextColor);
        }

    noOfDifferentColors = colorsPresentInNextMatrix.count();
    //urmatoarea saptamana voi avea noOfDifferentColors culori diferite in sala

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

    //citesc matricea

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
