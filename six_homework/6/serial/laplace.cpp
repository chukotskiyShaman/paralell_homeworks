#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "laplace.h"
#include <omp.h>

#define OFFSET(x, y, n)(((x)*(n)) + (y))

Laplace::Laplace(int n) : n(n){
    A = new double[n*n];
    Anew = new double[n*n];
    memset(A, 0, n*n*sizeof(double));
    memset(Anew, 0, n*n*sizeof(double));

    double corners[4] = {10, 20, 30, 20};
    A[0] = corners[0];
    A[n-1] = corners[1];
    A[n*n -1] = corners[2];
    A[n*(n-1)] = corners[3];
    Anew[0] = corners[0];
    Anew[n-1] = corners[1];
    Anew[n*n -1] = corners[2];
    Anew[n*(n-1)] = corners[3];
    double step = (corners[1] - corners[0])/(n-1);

    for(int i = 1; i < n; i++){
        A[i] = corners[0]+i*step;
        A[n*i] = corners[0] + i*step;
        A[(n-1) + n*i] = corners[1] + i*step;
        A[n*(n-1) + i] = corners[3] + i*step;
        Anew[i] = corners[0] + i*step;
        Anew[n*i] = corners[0] + i*step;
        Anew[(n-1) + n*i] = corners[1] + i*step;
        Anew[n*(n-1) + i] = corners[3] + i*step;
    }
}

Laplace::~Laplace(){
    std::ofstream out("out.txt");
    out<<std::fixed << std::setprecision(5);
    for(int j = 0; j < n; j++){
        for(int i = 0; i < n; i++){
            out<<std::left<<std::setw(10)<<A[OFFSET(j, i, n)] << " ";
        }
        out << std::endl;
    }
    delete (A);
    delete (Anew);
}

void Laplace::calcNext(){
    for(int j = 1; j < n-1; j++){
        for(int i = 1; i < n-1; i++){
            Anew[OFFSET(j,i,n)] = 0.25 * (A[OFFSET(j, i+1, n)] + A[OFFSET(j, i-1,n)] + A[OFFSET(j-1,i,n)] + A[OFFSET(j+1, i, n)]);
        }
    }
}

double Laplace::error_calc(){
    double error = 0.0;
    for(int j = 1; j < n-1; j++){
        for(int i = 1; i < n-1; i++){
            error = fmax(error, fabs(Anew[OFFSET(j,i,n)] - A[OFFSET(j,i,n)]));
        }
    }
    return error;
}

void Laplace::swap(){
    double* temp = A;
    A = Anew;
    Anew = temp;
    return;
}