#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "laplace.h"
#include <omp.h>

#define OFFSET(x, y, n) (((x)*(n)) + (y))

void initialize(double* A, double* Anew, int n)
{
    memset(A, 0, n * n * sizeof(double));
    memset(Anew, 0, n * n * sizeof(double));

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

double calcNext(double* A, double* Anew, int n)
{

    #pragma acc data present(A, Anew)
    #pragma acc parallel loop independent collapse(2) vector vector_length(256) gang num_gangs(256) async(1)
    for( int j = 1; j < n-1; j++)
    {
        for( int i = 1; i < n-1; i++ )
        {
            Anew[OFFSET(j, i, n)] = 0.25 * ( A[OFFSET(j, i+1, n)] + A[OFFSET(j, i-1, n)]
                                           + A[OFFSET(j-1, i, n)] + A[OFFSET(j+1, i, n)]);
        }
    }
}
void deallocate(double* A, double* Anew, int n)
{
    std::ofstream out("out.txt");
    out << std::fixed << std::setprecision(6);
    #pragma acc exit data copyout(A[:n*n], Anew[:n*n])
    for (int j = 0; j < n; j++){
        for (int i = 0; i < n; i++){
            out << std::left << std::setw(10) << A[OFFSET(j, i, n)] << " ";
        }
        out << std::endl;
    }
    #pragma acc exit data delete(A[ : n * n], Anew[ : n * n])
}