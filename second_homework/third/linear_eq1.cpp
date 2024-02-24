#include <iostream>
#include <vector>
#include <omp.h>
#include <time.h>

std::vector<double> solveLinearSystem(std::vector<std::vector<double>> A, std::vector<double> b){
    int n = A.size();
    for(int i = 0; i < n; i++){
        double div = A[i][i];
        for(int j = 0; j < n; j++){
            A[i][j] /=div;
        }
        b[i] /= div;
        for(int k = 0; k < n; k++){
            if(k!=i){
                double mult = A[k][i];
                for (int l = 0; l < n; l++){
                    A[k][l] -= mult * A[i][l];
                }
                b[k] -= mult * b[i];
            }
        }
    }
    return b;
} 
double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

int main(int argc, char**argv){
    int num_threads = 2;
    std::vector<std::vector<double>> A = {{2,1,1,1,1,1,1,1,1}, {1,2,1,1,1,1,1,1,1}, {1,1,2,1,1,1,1,1,1},{1,1,1,2,1,1,1,1,1},{1,1,1,1,2,1,1,1,1},{1,1,1,1,1,2,1,1,1},{1,1,1,1,1,1,2,1,1},{1,1,1,1,1,1,1,2,1},{1,1,1,1,1,1,1,1,2}};
    std::vector<double> b = {10,10,10,10,10,10,10,10,10};

    double t = cpuSecond();

    std::vector<double> x = solveLinearSystem(A, b);
    t = cpuSecond() - t;
    std::cout<< "Время выполнения команды: " << t << std::endl;
    std::cout << "Решение: "<< std::endl;
    for(double val : x){
        std::cout << val << " ";
    }
    std::cout<<std::endl;
    return 0;
}