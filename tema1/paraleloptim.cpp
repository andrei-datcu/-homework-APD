/*
 * Datcu Andrei Daniel 331CC
 * Tema1 APD
 * Rezolvarea problemei paralel, optimizat
 * Timp de executie O(s * n ^ 3)
 *
 * 08.11.2013
 *
 * NOTA: Aceasta sursa nu contine comentarii, fiind foarte asemanatoare
 * cu varianta sa paralela. Pentru mai multe comentarii vezi README
 * si serialoptim.cpp
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
typedef std::list<senator> ColorList[NCMAX];


void simweek(int nc, int n, ColorList &colors){


    static const int dc[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    static const int dl[8] = {1, 1, 0, -1, -1, -1, 0, 1};


    static char situation[NCMAX][NMAX][NMAX];

    #pragma omp parallel for schedule(dynamic) \
        shared(n, colors, dc, dl, situation)
    for (int i = 0; i < nc; ++i)
        if (!colors[i].empty()){// culoarea inca mai exista pe scena politica

            senator curent, vecin;
            std::queue<senator> searchQueue;
            senator father[NMAX][NMAX];
            char twsituation[NMAX][NMAX] = {{0}};

            for (senator &s : colors[i]){
                searchQueue.push(s);
                twsituation[s.first][s.second] = MAXDISTANCE;
                father[s.first][s.second] = s;
            }


            while (!searchQueue.empty()){
                curent = searchQueue.front();

                for (int k = 0; k < 8; ++k){
                    vecin = std::make_pair(curent.first + dl[k],
                            curent.second + dc[k]);

                    if (vecin.first >= 0 && vecin.first <= (n - 1) &&
                            vecin.second >= 0 && vecin.second <= (n - 1))

                        if (situation[i][vecin.first][vecin.second] == 0  &&
                                twsituation[vecin.first][vecin.second] == 0){

                            searchQueue.push(vecin);
                            situation[i][vecin.first][vecin.second] =
                              situation[i][curent.first][curent.second]  + 1;
                            father[vecin.first][vecin.second] =
                                father[curent.first][curent.second];
                        }
                        else{

                            senator f_vecin =father[vecin.first][vecin.second],
                                f_curent = father[curent.first][curent.second];

                            if (f_curent != f_vecin){

                                int distance = situation[i][vecin.first]
                                    [vecin.second] + situation[i]
                                    [curent.first][curent.second] + 1;

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

            for (senator s: colors[i])
                situation[i][s.first][s.second] =
                    twsituation[s.first][s.second] == MAXDISTANCE ? 0 :
                     twsituation[s.first][s.second];

        }

    ColorList temp;


    #pragma omp parallel for \
        shared(n, nc, situation, temp)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j){

            int max = 0, nextColor = i;
            for (int k = 0; k < nc; ++k){
                if (max < situation[k][i][j]){
                    max = situation[k][i][j];
                    nextColor = k;
                }
                situation[k][i][j] = 0;
            }
            #pragma omp critical
            temp[nextColor].push_back(std::make_pair(i, j));
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

    for (int i = 0; i < n; ++i){
        for (int j = 0; j < n; ++ j)
            fout << matrix[i][j] << " ";

        fout << std::endl;
    }

    fout.close();

    return 0;
}
