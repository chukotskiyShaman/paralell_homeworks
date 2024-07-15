#include <iostream>
#include <omp.h>
#include <new>
#include <nvtx3/nvToolsExt.h>
#include <chrono>
#include <boost/program_options.hpp>
#include <cuda_runtime.h>
#include "cublas_v2.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>

#define OFFSET(x, y, m) (((x) * (m)) + (y))


void initFunc(double* A, double* Anew, int n, int m);


namespace po = boost::program_options;


int main(int argc, char **argv) {
    int m = 4096;
    int n;
    int iter_max = 1000;
    double tol = 1.0e-6;

    double error = 1.0;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Usage: set precision by running -p 123, set grid size by using -n 123 and set maximum number of iterations by -i 123")
        ("precision,p", po::value<double>(&tol)->default_value(1.0e-6), "precision")
        ("grid_size,n", po::value<int>(&m)->default_value(4096), "grid size")
        ("iterations,i", po::value<int>(&iter_max)->default_value(1000), "number of iterations");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    n = m;

    double* A = new double[n * m];
    double* Anew = new double[n * m];
    memset(A, 0, n * m * sizeof(double));
    memset(Anew, 0, n * m * sizeof(double));
    initFunc(A, Anew, n, m);
    #pragma acc enter data copyin(A[ : m * n], Anew[ : m * n])

    printf("Jacobi relaxation Calculation: %d x %d mesh\n", m, m);

    auto start = std::chrono::high_resolution_clock::now();
    int iter = 0;

    nvtxRangePushA("while");
    while (error > tol && iter < iter_max)
    {
        nvtxRangePushA("calc");
        #pragma acc parallel loop present(A, Anew)
        for (int j = 1; j < n  - 1; j++){
            #pragma acc loop
            for (int i = 1; i < m - 1; i++){
                Anew[OFFSET(j, i, m)] = 0.25 * (A[OFFSET(j, i + 1, m)] + A[OFFSET(j, i - 1, m)] + A[OFFSET(j - 1, i, m)] + A[OFFSET(j + 1, i, m)]);
            }
        }
        nvtxRangePop();
        
        if (iter % 1000 == 0){
            nvtxRangePushA("error");
            error = 0.0;
            #pragma acc parallel loop reduction(max : error) present(A, Anew)
            for (int j = 1; j < n - 1; j++){
                #pragma acc loop 
                for (int i = 1; i < m - 1; i++){
                    error = fmax(error, fabs(Anew[OFFSET(j, i, m)] - A[OFFSET(j, i, m)]));
                }
            }
            nvtxRangePop();
            printf("%5d, %0.6f\n", iter, error);
        }
        
        nvtxRangePushA("swap");
        double *temp = A;
        A = Anew;
        Anew = temp;
        #pragma acc data present(A, Anew)
        nvtxRangePop();
        iter++;
    }
    nvtxRangePop();


    auto end = std::chrono::high_resolution_clock::now();
    auto duration_sec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);



    std::cout << " total: " << duration_sec.count() << " ms\n";

    std::ofstream out("out.txt");
    out << std::fixed << std::setprecision(5);
    #pragma acc update host(A[:n * m])
    for (int j = 0; j < n; j++){
        for (int i = 0; i < m; i++){
            out << std::left << std::setw(10) << A[OFFSET(j, i, m)] << " ";
        }
        out << std::endl;
    }

    #pragma acc exit data delete (A[:m * n], Anew[:m * n])
    delete[] A;
    delete[] Anew;

    return 0;
}

void initFunc(double* A, double* Anew, int n, int m){


    double corners[4] = {10, 20, 30, 20};
    double step = (corners[1] - corners[0]) / (n - 1);

    int lastIdx = n - 1;
    A[0] = Anew[0] = corners[0];
    A[lastIdx] = Anew[lastIdx] = corners[1];
    A[n * lastIdx] = Anew[n * lastIdx] = corners[3];
    A[n * n - 1] = Anew[n * n - 1] = corners[2];

    for (int i = 1; i < n - 1; i++)
    {
        double val = corners[0] + i * step;
        A[i] = Anew[i] = val;                       
        A[n * i] = Anew[n * i] = val;               
        A[lastIdx + n * i] = Anew[lastIdx + n * i] = corners[1] + i * step;
        A[n * lastIdx + i] = Anew[n * lastIdx + i] = corners[3] + i * step; 
    }
}