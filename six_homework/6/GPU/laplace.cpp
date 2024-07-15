#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "laplace.h"
#include <omp.h>

#define OFFSET(x, y, n) (((x)*(n)) + (y))

Laplace::Laplace(int n) : n(n){
    A = new double[n * n];
    Anew = new double[n * n];
    memset(A, 0, n * n * sizeof(double));
    memset(Anew, 0, n * n * sizeof(double));
    #pragma acc enter data copyin(this) 
    double corners[4] = {10, 20, 30, 20};
    A[0] = corners[0];
    A[n - 1] = corners[1];
    A[n * n - 1] = corners[2];
    A[n * (n - 1)] = corners[3];
    Anew[0] = corners[0];
    Anew[n - 1] = corners[1];
    Anew[n * n - 1] = corners[2];
    Anew[n * (n - 1)] = corners[3];
    double step = (corners[1] - corners[0]) / (n - 1);
    for (int i = 1; i < n - 1; i ++) {
        A[i] = corners[0] + i * step;
        A[n * i] = corners[0] + i * step;
        A[(n-1) + n * i] = corners[1] + i * step;
        A[n * (n-1) + i] = corners[3] + i * step;
        Anew[i] = corners[0] + i * step;
        Anew[n * i] = corners[0] + i * step;
        Anew[(n-1) + n * i] = corners[1] + i * step;
        Anew[n * (n-1) + i] = corners[3] + i * step;
    
    }
    #pragma acc enter data copyin(A[ : n * n], Anew[ : n * n])
}


Laplace::~Laplace(){
    std::ofstream out("out.txt");
    out << std::fixed << std::setprecision(5);
    #pragma acc exit data copyout(A[:n*n], Anew[:n*n])
    #pragma acc exit data copyout(this)
    for (int j = 0; j < n; j++){
        for (int i = 0; i < n; i++){
            out << std::left << std::setw(10) << A[OFFSET(j, i, n)] << " ";
        }
        out << std::endl;
    }
    #pragma acc exit data delete(A[ : n * n], Anew[ : n * n])
    #pragma acc exit data delete (this)
    delete (A);
    delete (Anew);
}


void Laplace::calcNext(){
    // #pragma acc data present(A, Anew)
    // #pragma acc parallel loop independent collapse(2) vector vector_length(256) gang num_gangs(256)
    // for (int j = 1; j < n - 1; j++){
    //     for (int i = 1; i < n - 1; i++){
    //         Anew[OFFSET(j, i, n)] = 0.25 * (A[OFFSET(j, i + 1, n)] + A[OFFSET(j, i - 1, n)] + A[OFFSET(j - 1, i, n)] + A[OFFSET(j + 1, i, n)]);
    //     }
    // }
    #pragma acc parallel loop present(A, Anew)
        for (int j = 1; j < n  - 1; j++){
            #pragma acc loop
            for (int i = 1; i < n - 1; i++){
                Anew[OFFSET(j, i, n)] = 0.25 * (A[OFFSET(j, i + 1, n)] + A[OFFSET(j, i - 1, n)] + A[OFFSET(j - 1, i, n)] + A[OFFSET(j + 1, i, n)]);
            }
        }
}

double Laplace::error_calc(){
    double error = 0.0;
    // #pragma acc enter data copyin(error)
    #pragma acc data present(A, Anew)//, error
    #pragma acc parallel loop independent collapse(2) reduction(max : error) vector vector_length(256) gang num_gangs(256)
    for (int j = 1; j < n - 1; j++){
        for (int i = 1; i < n - 1; i++){
            error = fmax(error, fabs(Anew[OFFSET(j, i, n)] - A[OFFSET(j, i, n)]));
        }
    }
    // #pragma acc exit data copyout(error)
    return error;
}

void Laplace::swap(){
    double* temp = A;
    A = Anew;
    Anew = temp;
}