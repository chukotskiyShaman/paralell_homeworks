#include <iostream>
#include "laplace.h"
#include <chrono>
#include <boost/program_options.hpp>
#include <omp.h>
#include <cublas_v2.h>


namespace po = boost::program_options;

int main(int argc, char** argv)
{
    int n = 4096;
    int iter_max = 1000;

    double tol = 1.0e-6;
    double error = 1.0;

    po::options_description desc("Options");

    desc.add_options()
        ("help,h", "Usage: -p - precision(0.001), size(10,16,32,1024), number of itterations(from 1 to 100000)")
        ("precision,p", po::value<double>(&tol)->default_value(1.0e-6), "precision")
        ("size,n", po::value<int>(&n)->default_value(128), "size")
        ("iterations,i", po::value<int>(&iter_max)->default_value(1000000), "number of iterations");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")){ std::cout << desc << '\n'; return 0;}
    po::notify(vm);
    if (iter_max>1e+6){
        std::cout<<"too much iterations\n";
        exit(1);
    }
    if (tol<1e-6){
        std::cout<<"too big precision\n";
        exit(1);
    }
    std::unique_ptr<double[]> A_ptr(new double[n*n]);
    std::unique_ptr<double[]> Anew_ptr(new double[n*n]);

    double* A = A_ptr.get();
    double* Anew = Anew_ptr.get();
    
    initialize(A, Anew,n);


    auto start = std::chrono::high_resolution_clock::now();
    int iter = 0;
    int idx=0;
    double alpha = -1.0;

    cublasHandle_t handler;
    cublasStatus_t stat;
    stat = cublasCreate(&handler);

    while ( error > tol && iter < iter_max )
    {
        calcNext(A, Anew,n);



        if(iter % 100 == 0){
            #pragma acc data present(A, Anew) wait
			#pragma acc host_data use_device(A, Anew)
			{
				stat = cublasDaxpy(handler, n * n, &alpha, Anew, 1, A, 1);
				stat = cublasIdamax(handler, n * n, A, 1, &idx);
			}

			#pragma acc update host(A[idx - 1]) 
			error = std::abs(A[idx - 1]);

			#pragma acc host_data use_device(A, Anew)
			stat = cublasDcopy(handler, n * n, Anew, 1, A, 1);
            std::cout<<iter<<", "<< error<<"\n";

        }

        double* temp = A;
        A = Anew;
        Anew = temp;

        iter++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto spent_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout<<" total: "<< iter <<" " << spent_time.count() << " ms \n";

    deallocate(A, Anew, n);
    cublasDestroy(handler);

    return 0;
}