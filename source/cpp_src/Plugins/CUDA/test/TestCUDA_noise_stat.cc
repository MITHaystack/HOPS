#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <iomanip>
#include <random>
#include <cmath>

// CUDA includes
#include <cuComplex.h>
#include <cufft.h>
#include <stdint.h>
#include <cuda_runtime_api.h>
#include <cuda.h>

#include "noise_statistics_mbp_reduce.h"

int main(int /*argc*/, char** /*argv*/)
{
    int n = 25165824;//100000000;

    // Host input vectors
    float* h_a;
    float* h_sum;
    float* h_sum2;

    // Allocate memory for each vector on host
    h_a = new float[n];
    h_sum = new float[ 1024];
    h_sum2 = new float[ 1024];

    //random number generator
    double mean = 0;
    double sigma = 4.0;
    std::random_device r_dev;
    std::mt19937 mt_generator( r_dev() );
    std::normal_distribution< float > r_dist(mean, sigma);

    for(size_t i=0; i<n; i++)
    {
        h_a[i] = r_dist( mt_generator );
    }


    // int i;
    // // Initialize vectors on host
    // for( i = 0; i < n; i++ )
    // {
    //     h_a[i] = 2.0;//i; //1.0;
    // }

    noise_statistics_mbp_reduce(h_a, h_sum, h_sum2, n);

    std::cout << std::setprecision(14)<<std::endl;

    float result = ( (float)n*( ( (float)n+1.0) ))/2.0;
    std::cout<< "expected result : " << result <<std::endl;
    std::cout<< "sum result : " << h_sum[0] <<std::endl;
    std::cout<< "sum2 result : " << h_sum2[0] <<std::endl;

    float m = h_sum[0]/(float)n;
    float e2 =  h_sum2[0]/(float)n;

    float std_dev = std::sqrt( e2 - m*m );

    std::cout<<"expected std dev = "<<sigma<<std::endl;
    std::cout<<"calculated std dev = "<<std_dev<<std::endl;

    delete[] h_a;
    delete[] h_sum;
    delete[] h_sum2;

    return 0;
}
