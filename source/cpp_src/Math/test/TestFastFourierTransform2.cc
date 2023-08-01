#include "MHO_Message.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_FastFourierTransformWorkspace.hh"
#include "MHO_FastFourierTransformCalls.hh"
#include "MHO_TestAssertions.hh"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

typedef double FP_Type;

int main(int /*argc*/, char** /*argv*/)
{
    std::vector< unsigned int > fftSizes;
    fftSizes = {32, 21, 79, 256, 1024, 8192, 16384};
    
    for(unsigned int i=0; i<fftSizes.size(); i++)
    {
        unsigned int N = fftSizes[i];
        
        std::vector< std::complex<FP_Type> > arr;
        std::vector< std::complex<FP_Type> > arr_orig;
        arr.resize(N);
        arr_orig.resize(N);
        
        //fill up the test arrays 
        for(unsigned int j=0; j<N; j++)
        {
            arr[j] = j%13 + j%17;
            arr_orig[j] = j%13 + j%17;
        }
        
        std::complex<FP_Type>* data = &(arr[0]);
        MHO_FastFourierTransformWorkspace<FP_Type> work;
        work.Resize(N);

        //forward transform 
        bool isForward = true;
        if( work.IsRadix2() )
        {
            FFTRadix2(data, work, isForward);
        }
        else 
        {
            FFTBluestein(data, work, isForward);
        }
        
        //reverse the FFT
        isForward = false;
        if( work.IsRadix2() )
        {
            FFTRadix2(data, work, isForward);
        }
        else 
        {
            FFTBluestein(data, work, isForward);
        }
        
        //compute the L2 norm difference so we have a single value to test against
        std::complex<double> delta;
        double sum2 = 0.0;
        for (unsigned int i = 0; i < N; i++)
        {
            //normalize
            arr[i] *= 1.0 / ((FP_Type) N);
            //take difference
            delta = (arr_orig[i] - arr[i]);
            sum2 += std::abs(delta)*std::abs(delta);
        }
        double sum = std::sqrt(sum2);
        
        std::cout<<"FFT of length: "<<N<<std::endl;
        std::cout<<"L2_diff = "<<sum<<std::endl;
        std::cout<<"L2_diff/N = "<<sum/N<<std::endl;

        double tol = 1.5e-15; //tested for N=256
        sum /= (double)N;

        HOPS_ASSERT_FLOAT_LESS_THAN(sum, tol);
    }

    return 0;
}
