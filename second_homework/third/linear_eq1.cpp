#include <iostream>
#include <vector>
#include <omp.h>
#include <time.h>

#define variable (double)1/N

const int N = 1000; // Размер системы
int num_threads = 8;//Количество потоков


double cpuSecond(){
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

double find_optim(double *v){
    
}

double* find_solution(double *A, double *b){
    double *x = new double[N];

        double norm_b = find_norm(b);
        double norm_sub;
        double *sub;
        sub=new double[N];
        while(true){

            double *c;
            c=new double[N];

            matrix_vector_product_omp(A,x,c,N,N);

            #pragma omp for schedule(static)
            for(int i=0; i<N;i++){
                sub[i]=c[i] - b[i];
            }

            norm_sub=find_norm(sub);

            // std::cout<<norm_sub<<'/'<<norm_b<<'='<<norm_sub/norm_b<<'\n';
            // std::cout<<norm_sub<<'/'<<norm_b<<'='<<norm_sub/norm_b<<'\n';


            if(norm_sub/norm_b<0.0000001)
                break;
            
            #pragma omp for schedule(static)
            for (int i = 0; i < N; i++)
                sub[i] *= variable;

            #pragma omp for schedule(static)
            for (int i = 0; i < N; i++)
                x[i] -= sub[i];
                
        }
    // }
    return x;
}

int main() {
    double *A;
    A = new double[N*N];
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if(i == j){
                A[i*N + j] = 2;
            }
            else{
                A[i*N + j] = 1;
            }
        }
    }
    double *b;
    b = new double[N];
    for(i = 0; i < N; i++){
        b[i] = N+1;
    }
    double *x;
    x = new double[N];
    double time = cpuSecond();

//     std::vector<double> x(N, 0.0);
//     std::vector<double> b(N+1, N+1);

//     double time = cpuSecond();
// #pragma omp paralell for num_threads(40)
//     for (int iter = 0; iter < 1000; ++iter) {
//         std::vector<double> x_new(N, 0.0);
//         for (int i = 0; i < N; ++i) {
//             x_new[i] = (b[i] - 1.0) / 2.0;
//             for (int j = 0; j < N; ++j) {
//                 if (j != i) {
//                   x_new[i] += x[j] / 2.0;
//                 }   
//             }       
//         }
//         x = x_new;
//     }
//     time = cpuSecond() - time;
//     std::cout << "Время выполнения программы: " << time << std::endl;
//     std::cout << "Решение системы:" << std::endl;
//     for (int i = 0; i < N; ++i) {
//         std::cout << "x[" << i << "] = " << x[i] << std::endl;
//     }

    return 0;
}
