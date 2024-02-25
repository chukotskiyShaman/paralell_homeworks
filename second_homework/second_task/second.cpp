#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <cmath>

const double PI = 3.14159265358979323846;
const double a = -4.0;
const double b = 4.0;
const int nsteps = 40000000;
int num_threads = 2;

// std::string executeCommand(const std::string& command){
//     std::string result;
//     char buffer[128];
//     FILE* pipe = popen(command.c_str(), "r");
//     if(pipe){
//         while(!feof(pipe)){
//             if(fgets(buffer, 128, pipe) != nullptr)
//                 result += buffer;
//         }
//         pclose(pipe);
//     }
//     return result;
// }

// std::string getCPUInfo(){
//     return executeCommand("lscpu");
// }

// std::string getServerName(){
//     return executeCommand("cat /sys/devices/virtual/dmi/id/product_name");
// }

// std::string getNumaNodes(){
//     return executeCommand("numactl --hardware");
// }

// std::string getOSInfo(){
//     return executeCommand("cat /etc/os-release");
// }

// double cpuSecond()
// {
//     struct timespec ts;
//     timespec_get(&ts, TIME_UTC);
//     return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
// }

double func(double x)
{
    return exp(-x * x);
}

double integrate(double (*func)(double), double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = 0.0;

    for (int i = 0; i < n; i++)
        sum += func(a + h * (i + 0.5));

    sum *= h;

    return sum;
}

double integrate_omp(double (*func)(double), double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = 0.0;
    // std::cout<<"Num threads: "<<num_threads<<std::endl;

#pragma omp parallel num_threads(10)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);
        double sumloc = 0.0;
        for (int i = lb; i <= ub; i++)
            sumloc += func(a + h * (i + 0.5));
        #pragma omp atomic
        sum += sumloc;
    }
    sum *= h;
    return sum;
}

double run_serial()
{
    double start = omp_get_wtime();
    double res = integrate(func, a, b, nsteps);
    printf("Result (serial): %.12f; error %.12f\n", res, fabs(res - sqrt(PI)));
    double end = omp_get_wtime();
    return (end-start);
}
double run_parallel()
{
    double start = omp_get_wtime();
    double res = integrate_omp(func, a, b, nsteps);
    printf("Result (parallel): %.12f; error %.12f\n", res, fabs(res - sqrt(PI)));
    double end = omp_get_wtime();
    return (end-start);
}

int main(){//int argc, char **argv
    // num_threads = 2;
    // std::cout << "CPU Info: \n" << getCPUInfo() <<std::endl;
    // std::cout << "Server Name:\n" << getServerName() << std::endl;
    // std::cout << "NUMA Nodes:\n" << getNumaNodes() << std::endl;
    // std::cout << "OS Info:\n" << getOSInfo() << std::endl;
    // if(argc == 2){
    //     num_threads = atoi(argv[1]);
    // }

    printf("Integration f(x) on [%.12f, %.12f], nsteps = %d\n", a, b, nsteps);
    double tserial = run_serial();
    double tparallel = run_parallel();

    printf("Execution time (serial): %.6f\n", tserial);
    printf("Execution time (parallel): %.6f\n", tparallel);
    printf("Speedup: %.2f\n", tserial / tparallel);
    return 0;
}