#include "MHO_Message.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_TestAssertions.hh"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

typedef double FP_Type;

int main(int /*argc*/, char** /*argv*/)
{
    const unsigned int N = 256;
    unsigned int index_arr[N];
    std::complex<FP_Type> arr[N];
    std::complex<FP_Type> arr_orig[N];
    std::complex<FP_Type> twiddle[N];

    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "testing DIT followed by DIF (with bit-reversal permutation)" << std::endl;
    #endif

    MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(N, index_arr);
    //fill up the array with a signal
    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "Original array = " << std::endl;
    #endif
    for (unsigned int i = 0; i < N; i++)
    {
        arr[i] = std::complex<FP_Type>(i, 0);
        arr_orig[i] = arr[i];
        #ifdef HOPS_ENABLE_DEBUG_MSG
        std::cout << arr[i] << std::endl;
        #endif
    }

    //compute twiddle factors
    MHO_FastFourierTransformUtilities<FP_Type>::ComputeTwiddleFactors(N, twiddle);

    //scrambles the array in opposite sense of the way the FFT scrambles
    MHO_BitReversalPermutation::PermuteArray<std::complex<FP_Type>>(N, index_arr, arr);

    //do the radix-2 FFT decimation in time
    MHO_FastFourierTransformUtilities<FP_Type>::FFTRadixTwo_DIT(N, (FP_Type*) &(arr[0]), (FP_Type*) &(twiddle[0]));

    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "(unormalized DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++)
    {
        std::cout << arr[i] << std::endl;
    }
    #endif

    //now we'll do the inverse transform
    //get conjugate twiddle factors
    MHO_FastFourierTransformUtilities<FP_Type>::ComputeConjugateTwiddleFactors(N, twiddle);
    //do the radix-2 FFT decimation in frequency
    MHO_FastFourierTransformUtilities<FP_Type>::FFTRadixTwo_DIF(N, (FP_Type*) &(arr[0]), (FP_Type*) &(twiddle[0]));
    //unscramble the array
    MHO_BitReversalPermutation::PermuteArray<std::complex<FP_Type>>(N, index_arr, arr);

    //normalize
    for (unsigned int i = 0; i < N; i++)
    {
        arr[i] *= 1.0 / ((FP_Type) N);
    }

    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "difference between original and IDFT of the DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        std::cout << arr_orig[i] - arr[i] << std::endl;
    }
    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "testing DIF followed by DIT (without bit-reversal permutation)" << std::endl;
    #endif

    //fill up the array with a signal
    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "Original array = " << std::endl;
    #endif
    for (unsigned int i = 0; i < N; i++) {
        arr[i] = std::complex<FP_Type>(i, 0);
        arr_orig[i] = arr[i];
        #ifdef HOPS_ENABLE_DEBUG_MSG
        std::cout << arr[i] << std::endl;
        #endif
    }

    //compute twiddle factors
    MHO_FastFourierTransformUtilities<FP_Type>::ComputeTwiddleFactors(N, twiddle);
    //do the radix-2 FFT
    MHO_FastFourierTransformUtilities<FP_Type>::FFTRadixTwo_DIF(N, (FP_Type*) &(arr[0]), (FP_Type*) &(twiddle[0]));

    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "unormalized DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        std::cout << arr[i] << std::endl;
    }
    #endif

    //now we'll do the inverse transform
    //simpler to just conjugate the twiddle factors
    for (unsigned int i = 0; i < N; i++)
    {
        twiddle[i] = std::conj(twiddle[i]);
    }
    //do the radix-2 FFT
    MHO_FastFourierTransformUtilities<FP_Type>::FFTRadixTwo_DIT(N, (FP_Type*) &(arr[0]), (FP_Type*) &(twiddle[0]));
    //normalize
    for (unsigned int i = 0; i < N; i++)
    {
        arr[i] *= 1.0 / ((FP_Type) N);
    }

    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "difference between original and IDFT of the DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++)
    {
        std::cout << arr_orig[i] - arr[i] << std::endl;
    }
    #endif

    //compute the L2 norm difference so we have a single value to test against
    std::complex<double> delta;
    double sum2 = 0.0;
    for (unsigned int i = 0; i < N; i++)
    {
        delta = (arr_orig[i] - arr[i]);
        sum2 += std::abs(delta)*std::abs(delta);
    }
    double sum = std::sqrt(sum2);

    std::cout<<"L2_diff = "<<sum<<std::endl;
    std::cout<<"L2_diff/N = "<<sum/N<<std::endl;

    double tol = 1.5e-15; //tested for N=256
    sum /= (double)N;

    HOPS_ASSERT_FLOAT_LESS_THAN(sum, tol);

    return 0;
}
