#include "MHOMessage.hh"
#include "MHOBitReversalPermutation.hh"
#include "MHOFastFourierTransformUtilities.hh"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    const unsigned int N = 8;
    unsigned int index_arr[N];
    std::complex<double> arr[N];
    std::complex<double> arr_orig[N];
    std::complex<double> twiddle[N];

    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "testing DIT followed by DIF (with bit-reversal permutation)" << std::endl;


    MHOBitReversalPermutation::ComputeBitReversedIndicesBaseTwo(N, index_arr);
    //fill up the array with a signal
    std::cout << "Original array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        arr[i] = std::complex<double>(i, 0);
        arr_orig[i] = arr[i];
        std::cout << arr[i] << std::endl;
    }

    //compute twiddle factors
    MHOFastFourierTransformUtilities::ComputeTwiddleFactors(N, twiddle);

    //scrambles the array in opposite sense of the way the FFT scrambles
    MHOBitReversalPermutation::PermuteArray<std::complex<double>>(N, index_arr, arr);

    //do the radix-2 FFT decimation in time
    MHOFastFourierTransformUtilities::FFTRadixTwo_DIT(N, (double*) &(arr[0]), (double*) &(twiddle[0]));

    std::cout << "(unormalized DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        std::cout << arr[i] << std::endl;
    }

    //now we'll do the inverse transform
    //get conjugate twiddle factors
    MHOFastFourierTransformUtilities::ComputeConjugateTwiddleFactors(N, twiddle);
    //do the radix-2 FFT decimation in frequency
    MHOFastFourierTransformUtilities::FFTRadixTwo_DIF(N, (double*) &(arr[0]), (double*) &(twiddle[0]));
    //unscramble the array
    MHOBitReversalPermutation::PermuteArray<std::complex<double>>(N, index_arr, arr);

    //normalize
    for (unsigned int i = 0; i < N; i++) {
        arr[i] *= 1.0 / ((double) N);
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
        arr[i] = std::complex<double>(i, 0);
        arr_orig[i] = arr[i];
        std::cout << arr[i] << std::endl;
    }

    //compute twiddle factors
    MHOFastFourierTransformUtilities::ComputeTwiddleFactors(N, twiddle);
    //do the radix-2 FFT
    MHOFastFourierTransformUtilities::FFTRadixTwo_DIF(N, (double*) &(arr[0]), (double*) &(twiddle[0]));
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
    MHOFastFourierTransformUtilities::FFTRadixTwo_DIT(N, (double*) &(arr[0]), (double*) &(twiddle[0]));
    //normalize
    for (unsigned int i = 0; i < N; i++) {
        arr[i] *= 1.0 / ((double) N);
    }
    std::cout << "difference between original and IDFT of the DFT'd array = " << std::endl;
    for (unsigned int i = 0; i < N; i++) {
        std::cout << arr_orig[i] - arr[i] << std::endl;
    }


    return 0;
}
