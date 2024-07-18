#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <iomanip>

// CUDA includes
#include <cuComplex.h>
#include <cufft.h>
#include <stdint.h>
#include <cuda_runtime_api.h>
#include <cuda.h>

#include "vector_mbp_reduce.h"

int main(int /*argc*/, char** /*argv*/)
{
    int n = 25165824;//100000000;

    // Host input vectors
    float* h_a;
    float* h_c;

    // Allocate memory for each vector on host
    h_a = new float[n];
    h_c = new float[1];
    h_c[0] = 0.0;

    int i;
    // Initialize vectors on host
    for( i = 0; i < n; i++ )
    {
        h_a[i] = i; //1.0;
    }

    vector_mbp_reduce(h_a, h_c, n);

    std::cout << std::setprecision(14)<<std::endl;

    float result = ( (float)n*( ( (float)n+1.0) ))/2.0;
    std::cout<< "expected result : " << result <<std::endl;
    std::cout<< "final result : " << h_c[0] <<std::endl;

    delete[] h_a;
    delete[] h_c;

    return 0;
}
