#include <iostream>
#include <boost/program_options.hpp>
#include <omp.h>
#include <new>
#include <nvtx3/nvToolsExt.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <cuda_runtime.h>
#include <cub/cub.cuh>
#include <cub/block/block_load.cuh>
#include <cub/block/block_reduce.cuh>
#include <cub/block/block_store.cuh>
#define OFFSET(x, y, m) (((x) * (m)) + (y))


namespace po = boost::program_options;

// cuda unique_ptr
template<typename T>
using cuda_unique_ptr = std::unique_ptr<T,std::function<void(T*)>>;


void cudaCheck(cudaError_t error, char err_src[]) { //error printing function to reduce line count
    if (error != cudaSuccess) {
        printf("Error: %i while performing %s \n", error, err_src);
        exit(EXIT_FAILURE);
    }
}
// new
template<typename T>
T* cuda_new(size_t size)
{
    T *d_ptr;
    cudaError_t cudaErr = cudaSuccess;
    cudaErr = cudaMalloc((void **)&d_ptr, sizeof(T) * size);
    cudaCheck(cudaErr, "error during cudaMalloc");
    return d_ptr;
}

// delete
template<typename T>
void cuda_delete(T *dev_ptr)
{
    cudaFree(dev_ptr);
}


__global__ void subtractArrays(const double *A, const double *Anew, double *Sub_res , int m) {
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    int j = blockDim.y * blockIdx.y + threadIdx.y;
    if ((i >= 0) && (i < m) && (j >= 0) && (j < m)) {
        Sub_res[OFFSET(i,j,m)] = A[OFFSET(i,j,m)] - Anew[OFFSET(i,j,m)];
    }
}


__global__ void calcNext(double *A, double *Anew, int m, bool calcLeft) {
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    int j = blockDim.y * blockIdx.y + threadIdx.y;
    if (calcLeft){
        if ((i > 0) && (i < m - 1) && (j > 0) && (j < m - 1)) {
            A[OFFSET(j, i, m)] = 0.25 * (Anew[OFFSET(j, i + 1, m)] + Anew[OFFSET(j, i - 1, m)]
            + Anew[OFFSET(j - 1, i, m)] + Anew[OFFSET(j + 1, i, m)]);
        }
    }
    else{
        if ((i > 0) && (i < m - 1) && (j > 0) && (j < m - 1)) {
            Anew[OFFSET(j, i, m)] = 0.25 * (A[OFFSET(j, i + 1, m)] + A[OFFSET(j, i - 1, m)]
            + A[OFFSET(j - 1, i, m)] + A[OFFSET(j + 1, i, m)]);
        }
    }
}



int main(int argc, char **argv)
{
    int m = 256;
    int iter_max = 1000000;
    double tol = 1.0e-6;
    double error = 1.0;
    po::options_description desc("Options");
    desc.add_options()
        ("help", "print help")
        ("error,e", po::value<double>(&tol)->default_value(tol), "min error")
        ("size,n", po::value<int>(&m)->default_value(m), "size of grid")
        ("iterations,i", po::value<int>(&iter_max)->default_value(iter_max), "number of iterations");

    // Парсинг аргументов командной строки
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    int n = m;

    std::unique_ptr<double[]> A_ptr(new double[m*m]);
    std::unique_ptr<double[]> Anew_ptr(new double[m*m]);
    std::unique_ptr<double[]> Subtract_temp_ptr(new double[m*m]);

    double* A = A_ptr.get();
    double* Anew = Anew_ptr.get();
    double* Subtract_temp = Subtract_temp_ptr.get();

    nvtxRangePushA("init");
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
    nvtxRangePop();

    // размерности grid и block
    dim3 grid(32 , 32);
	dim3 block(n/32, n/32);

    cudaError_t cudaErr = cudaSuccess;
    cudaStream_t stream;
    cudaStreamCreate(&stream);
    cudaCheck(cudaErr, "error during cudaStreamCreate");

    cuda_unique_ptr<double> d_unique_ptr_error(cuda_new<double>(0), cuda_delete<double>);
    cuda_unique_ptr<void> d_unique_ptr_temp_storage(cuda_new<void>(0), cuda_delete<void>);

    cuda_unique_ptr<double> d_unique_ptr_A(cuda_new<double>(m*m), cuda_delete<double>);
    cuda_unique_ptr<double> d_unique_ptr_Anew(cuda_new<double>(m*m), cuda_delete<double>);
    cuda_unique_ptr<double> d_unique_ptr_Subtract_temp(cuda_new<double>(m*m), cuda_delete<double>);
    
    // выделение памяти и перенос на GPU
	double *d_error_ptr = d_unique_ptr_error.get();
	cudaErr = cudaMalloc((void**)&d_error_ptr, sizeof(double));


    double *d_A = d_unique_ptr_A.get();

	double *d_Anew = d_unique_ptr_Anew.get();

    double *d_Subtract_temp = d_unique_ptr_Subtract_temp.get();

    cudaErr = cudaMemcpyAsync(d_A, A, m*m*sizeof(double), cudaMemcpyHostToDevice, stream);
    cudaCheck(cudaErr, "error during cudaMemcpyAsync");
    cudaErr = cudaMemcpyAsync(d_Anew, Anew, m*m*sizeof(double), cudaMemcpyHostToDevice, stream);
    cudaCheck(cudaErr, "error during cudaMemcpyAsync");

	// проверка занимаемой памяти для редукции
    void *d_temp_storage = d_unique_ptr_temp_storage.get();
    size_t temp_storage_bytes = 0;
    cub::DeviceReduce::Max(d_temp_storage, temp_storage_bytes, d_Anew, d_error_ptr, m*m, stream);
    cudaMalloc((void**)&d_temp_storage, temp_storage_bytes);
    cudaCheck(cudaErr, "error during cudaMalloc");

    printf("temp_storage_bytes: %d\n", temp_storage_bytes);
    printf("Jacobi relaxation Calculation: %d x %d mesh\n", m, m);
    printf("Max iterations: %d\n", iter_max);
    printf("MIN Error: %lf\n\n", tol);

    // graph
    bool graph_created = false;
	cudaGraph_t graph;
	cudaGraphExec_t instance;

    int iter = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    nvtxRangePushA("while");
    while (error > tol && iter < iter_max)
    {
        if(!graph_created) {
            // создание графа
            nvtxRangePushA("createGraph");
            cudaErr = cudaStreamBeginCapture(stream, cudaStreamCaptureModeGlobal);
            cudaCheck(cudaErr, "error during cudaStreamBeginCapture");
            for (int i = 0; i < 100; i++) {
                calcNext<<<grid, block, 0, stream>>>(d_A, d_Anew, m, (bool)(i % 2));
            }
            cudaErr = cudaStreamEndCapture(stream, &graph);
            cudaCheck(cudaErr, "error during cudaStreamEndCapture");
            nvtxRangePop();
            cudaErr = cudaGraphInstantiate(&instance, graph, NULL, NULL, 0);
            cudaCheck(cudaErr, "error during cudaGraphInstantiate");
            graph_created = true;
        }
        nvtxRangePushA("startGraph");
        //запуск графа
        cudaErr = cudaGraphLaunch(instance, stream);
        cudaCheck(cudaErr, "error during cudaGraphLaunch");
        nvtxRangePop(); 
        iter += 100;
        if (iter % 100 == 0){
            nvtxRangePushA("calcError");
            subtractArrays<<<grid, block, 0, stream>>>(d_A, d_Anew, d_Subtract_temp, m);
            cub::DeviceReduce::Max(d_temp_storage, temp_storage_bytes, d_Subtract_temp, d_error_ptr, m*m, stream);
            cudaErr = cudaMemcpyAsync(&error, d_error_ptr, sizeof(double), cudaMemcpyDeviceToHost, stream);
            nvtxRangePop();
        }
        if (iter % 1000 == 0)
            printf("%5d, %0.6f\n", iter, error);
    }
    nvtxRangePop();
    printf("%5d, %0.6f\n", iter, error);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    printf("total: %f s\n", elapsed_seconds.count());
    cudaErr = cudaMemcpy(A, d_A, m*m*sizeof(double), cudaMemcpyDeviceToHost);
    cudaGraphDestroy(graph);
    std::ofstream out("out.txt");
    for (int j = 0; j < n; j++){
        for (int i = 0; i < m; i++){
            out << std::left << std::setw(10) << A[OFFSET(j, i, m)] << " ";
        }
        out << std::endl;
    }
    return 0;
}