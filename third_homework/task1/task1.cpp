#include <iostream>
#include <vector>
#include <thread>
#include <time.h>

const int N = 40000;
const int numThreads = 40;

std::vector<std::vector<double>> matrix(N, std::vector<double>(N));
std::vector<double> vector(N);
std::vector<double> res(N);

double cpuSecond(){
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void MVMult(int start, int end){
    for(int i = start; i < end; ++i){
        for(int j = 0; j < N; j++){
            res[i] += matrix[i][j] * vector[j];
        }
    }
}

int main(){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            matrix[i][j] = i * j;
        }
        vector[i] = i;
    }
    std::vector<std::thread> threads;
    int chunkSize = N / numThreads;
    for(int i = 0; i < numThreads; i++){
        int start = i*chunkSize;
        int end = (i==numThreads-1) ? N : (i+1)*chunkSize;
        threads.emplace_back(MVMult, start, end);
    }
    double time = cpuSecond();
    for(auto& thread : threads){
        thread.join();
    }
    time = cpuSecond() - time;
    std::cout << "Время на выполнение программы: " << time << std::endl; 
    return 0;
}