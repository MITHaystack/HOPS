#include "MHO_Message.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

typedef double FP_Type;

int main(int /*argc*/, char** /*argv*/)
{
    const unsigned int N = 8;
    unsigned int index_arr[N];
    std::complex<FP_Type> arr[N];
    std::complex<FP_Type> arr_orig[N];
    std::complex<FP_Type> twiddle[N];

    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "testing DIT followed by DIF (with bit-reversal permutation)" << std::endl;


    MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(N, index_arr);
    //fill up the array with a signal
    std::cout << "Original array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        arr[i] = std::complex<FP_Type>(i, 0);
        arr_orig[i] = arr[i];
        std::cout << arr[i] << std::endl;
    }

    //compute twiddle factors
    MHO_FastFourierTransformUtilities<FP_Type>::ComputeTwiddleFactors(N, twiddle);

    //scrambles the array in opposite sense of the way the FFT scrambles
    MHO_BitReversalPermutation::PermuteArray<std::complex<FP_Type>>(N, index_arr, arr);

    //do the radix-2 FFT decimation in time
    MHO_FastFourierTransformUtilities<FP_Type>::FFTRadixTwo_DIT(N, (FP_Type*) &(arr[0]), (FP_Type*) &(twiddle[0]));

    std::cout << "(unormalized DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        std::cout << arr[i] << std::endl;
    }

    //now we'll do the inverse transform
    //get conjugate twiddle factors
    MHO_FastFourierTransformUtilities<FP_Type>::ComputeConjugateTwiddleFactors(N, twiddle);
    //do the radix-2 FFT decimation in frequency
    MHO_FastFourierTransformUtilities<FP_Type>::FFTRadixTwo_DIF(N, (FP_Type*) &(arr[0]), (FP_Type*) &(twiddle[0]));
    //unscramble the array
    MHO_BitReversalPermutation::PermuteArray<std::complex<FP_Type>>(N, index_arr, arr);

    //normalize
    for (unsigned int i = 0; i < N; i++) {
        arr[i] *= 1.0 / ((FP_Type) N);
    }

    std::cout << "difference between original and IDFT of the DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        std::cout << arr_orig[i] - arr[i] << std::endl;
    }

    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "testing DIF followed by DIT (without bit-reversal permutation)" << std::endl;

    //fill up the array with a signal
    std::cout << "Original array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        arr[i] = std::complex<FP_Type>(i, 0);
        arr_orig[i] = arr[i];
        std::cout << arr[i] << std::endl;
    }

    //compute twiddle factors
    MHO_FastFourierTransformUtilities<FP_Type>::ComputeTwiddleFactors(N, twiddle);
    //do the radix-2 FFT
    MHO_FastFourierTransformUtilities<FP_Type>::FFTRadixTwo_DIF(N, (FP_Type*) &(arr[0]), (FP_Type*) &(twiddle[0]));
    std::cout << "unormalized DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        std::cout << arr[i] << std::endl;
    }

    //now we'll do the inverse transform
    //simpler to just conjugate the twiddle factors
    for (unsigned int i = 0; i < N; i++) {
        twiddle[i] = std::conj(twiddle[i]);
    }
    //do the radix-2 FFT
    MHO_FastFourierTransformUtilities<FP_Type>::FFTRadixTwo_DIT(N, (FP_Type*) &(arr[0]), (FP_Type*) &(twiddle[0]));
    //normalize
    for (unsigned int i = 0; i < N; i++) {
        arr[i] *= 1.0 / ((FP_Type) N);
    }
    std::cout << "difference between original and IDFT of the DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        std::cout << arr_orig[i] - arr[i] << std::endl;
    }


    return 0;
}
