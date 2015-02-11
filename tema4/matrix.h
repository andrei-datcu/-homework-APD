/*
 * Autor: Datcu Andrei Daniel
 * Grupa: 331CC
 * Data: 14.12.2013
 *
 * Tema 3 APD
 *
 * matrix.h
 *
 * Fisierul care implementeaza clasa Matrix.
 *
 * Clasa matrix definiste o matrice alocata dinamic de elemente de tip T
 *
 * Practic este un vector de nr_linii * nr_coloane.
 *
 * Operatorul [] intoarce un pointer la primul element de pe linia din
 * interiorul parantezelor. Astfel pot inlantui doi operatori [].
 *
 */

#ifndef __MATRIX__
#define __MATRIX__

template <typename T>
class Matrix{

public:

    Matrix(unsigned int lineCount, unsigned int colCount):
            lineCount(lineCount), colCount(colCount){
        data = new T[lineCount * colCount]();
    }

    ~Matrix(){
        delete[] data;
    }

    T* operator [](int line){
        return data + line * colCount;
    }

    Matrix<T>& operator |= (Matrix<T> &m){

        for (int i = 0; i < lineCount; ++i)
            for (int j = 0; j < colCount; ++j)
                (*this)[i][j] |= m[i][j];

        return *this;
    }

    T* getData(){

        return data;
    }

    const int lineCount, colCount;

private:
    T *data;
};
#endif
