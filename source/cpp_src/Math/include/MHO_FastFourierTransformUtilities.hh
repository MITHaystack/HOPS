#ifndef MHO_FastFourierTransformUtilities_HH__
#define MHO_FastFourierTransformUtilities_HH__

#include <cmath>
#include <complex>
#include <cstddef>

#include "MHO_BitReversalPermutation.hh"
#include "MHO_Message.hh"

namespace hops
{

/*!
 *@file MHO_FastFourierTransformUtilities.hh
 *@class  MHO_FastFourierTransformUtilities
 *@date Fri Oct 23 12:02:01 2020 -0400
 *@brief basic utility functions for native FFTs
 *@author J. Barrett - barrettj@mit.edu
 */

template< typename XFloatType > class MHO_FastFourierTransformUtilities
{
    public:
        MHO_FastFourierTransformUtilities(){};
        virtual ~MHO_FastFourierTransformUtilities(){};

        /**
         * @brief Conjugates each element in a complex array.
         *
         * @param N Size of the input array
         * @param array Input/output array of complex numbers
         * @note This is a static function.
         */
        static void Conjugate(unsigned int N, std::complex< XFloatType >* array)
        {
            for(unsigned int i = 0; i < N; i++)
            {
                array[i] = std::conj(array[i]);
            }
        }

        /**
         * @brief Conjugates each element in a complex (strided) array.
         *
         * @param N Size of the array
         * @param array Input/output complex array to conjugate
         * @param stride (unsigned int)
         * @note This is a static function.
         */
        static void Conjugate(unsigned int N, std::complex< XFloatType >* array, unsigned int stride)
        {
            for(unsigned int i = 0; i < N; i++)
            {
                array[i * stride] = std::conj(array[i * stride]);
            }
        }

        /**
         * @brief Compute twiddle factors for a Fast Fourier Transform.
         * computes all the twiddle factors e^{i*2*pi/N} for 0 to N-1
         * using std::cos and std::sin which is more accurate than the recursive method
         * @param N Size of the transform (N).
         * @param twiddle Output array to store computed twiddle factors.
         * @note This is a static function.
         */
        static void ComputeTwiddleFactors(unsigned int N, std::complex< XFloatType >* twiddle);

        /**
         * @brief Computes the conjugate twiddle factors for given size N and stores them in provided array.
         *
         * @param N Size of the transform (N must be a power of 2)
         * @param conj_twiddle Output array to store the computed conjugate twiddle factors
         * @note This is a static function.
         */
        static void ComputeConjugateTwiddleFactors(unsigned int N, std::complex< XFloatType >* conj_twiddle)
        {

            //to compute the twiddle factors
            ComputeTwiddleFactors(N, conj_twiddle);
            Conjugate(N, conj_twiddle);
        }

        ////////////////////////////////////////////////////////////////////////

        /**
         * @brief Computes twiddle factors for Fast Fourier Transform up to log2N.
         * computes only the twiddle factors we need in order to easily compute them on the fly
         * e.g e^{ix}, e^{2ix}, e^{4ix}, e^{8ix}, etc up to e^{Nix} ), that way we only need log2N storage
         * @param log2N Number of bits in N (N = 2^log2N)
         * @param twiddle Array to store computed twiddle factors
         * @note This is a static function.
         */
        static void ComputeTwiddleFactorBasis(unsigned int log2N, std::complex< XFloatType >* twiddle);

        /**
         * @brief Computes conjugate twiddle factor basis for given log2N and stores result in conj_twiddle.
         *
         * @param log2N Number of bits to divide N by 2 for twiddle factors
         * @param conj_twiddle (std::complex< XFloatType )*
         * @note This is a static function.
         */
        static void ComputeConjugateTwiddleFactorBasis(unsigned int log2N, std::complex< XFloatType >* conj_twiddle)
        {
            ComputeTwiddleFactors(log2N, conj_twiddle);
            Conjugate(log2N, conj_twiddle);
        }

        /**
         * @brief Computes a Radix-2 decimation in time (DIT) FFT.
         * input: data array in bit-address permutated order
         * output: fft of data in normal order
         * @param N Size of the data array (must be power of 2)
         * @param data Input/output array of complex numbers to conjugate
         * @param twiddle Array of precomputed twiddle factors
         * @param stride Stride between elements in the input array
         * @note This is a static function.
         */
        static void FFTRadixTwo_DIT(unsigned int N, XFloatType* data, XFloatType* twiddle, unsigned int stride = 1)
        {
            //decimation in time, N is assumed to be a power of 2
            unsigned int logN = MHO_BitReversalPermutation::LogBaseTwo(N);
            unsigned int butterfly_width;
            unsigned int n_butterfly_groups;
            unsigned int group_start;
            unsigned int butterfly_index;
            unsigned int access_stride = 2 * stride;
            XFloatType* H0;
            XFloatType* H1;
            XFloatType* W;

            for(unsigned int stage = 0; stage < logN; stage++)
            {
                //compute the width of each butterfly
                butterfly_width = MHO_BitReversalPermutation::TwoToThePowerOf(stage);
                //compute the number of butterfly groups
                n_butterfly_groups = N / (2 * butterfly_width);

                for(unsigned int n = 0; n < n_butterfly_groups; n++)
                {
                    //compute the starting index of this butterfly group
                    group_start = 2 * n * butterfly_width;
                    for(unsigned int k = 0; k < butterfly_width; k++)
                    {
                        butterfly_index = group_start + k; //index
                        H0 = data + (access_stride * butterfly_index);
                        H1 = H0 + access_stride * butterfly_width;
                        W = twiddle + 2 * n_butterfly_groups * k;
                        ButterflyRadixTwo_CooleyTukey(H0, H1, W);
                    }
                }
            }
        }

        /**
         * @brief Function ButterflyRadixTwo_CooleyTukey
         * See page 23 of "Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000
         * @param H0 (XFloatType*)
         * @param H1 (XFloatType*)
         * @param W (XFloatType*)
         * @note This is a static function.
         */
        static inline void ButterflyRadixTwo_CooleyTukey(XFloatType* H0, XFloatType* H1, XFloatType* W)
        {
            //H0 is the element from the even indexed array
            //H1 is the element from the odd index array
            //W is the twiddle factor

            //fetch the data
            XFloatType H00, H01, H10, H11, W0, W1, alpha_i, alpha_r;
            H00 = H0[0];
            H01 = H0[1];
            H10 = H1[0];
            H11 = H1[1];
            W0 = W[0];
            W1 = W[1];

            //apply the butterfly
            alpha_r = W0 * H10 - W1 * H11;
            alpha_i = W1 * H10 + W0 * H11;
            H10 = H00 - alpha_r;
            H11 = H01 - alpha_i;
            H00 = H00 + alpha_r;
            H01 = H01 + alpha_i;

            //write out
            H0[0] = H00;
            H0[1] = H01;
            H1[0] = H10;
            H1[1] = H11;
        }

        //RADIX-2 DIF
        /**
         * @brief Performs Radix-2 Decimation In Frequency (DIF) FFT using conjugate array and twiddle factors.
         * input: data array in normal order
         * output: fft of data in bit-address permutated order
         * @param N Size of the data array (must be power of 2)
         * @param data Input/output complex data array
         * @param twiddle Twiddle factor array
         * @param stride Stride for accessing data elements
         * @note This is a static function.
         */
        static void FFTRadixTwo_DIF(unsigned int N, XFloatType* data, XFloatType* twiddle, unsigned int stride = 1)
        {
            //decimation in frequency, N is assumed to be a power of 2
            unsigned int logN = MHO_BitReversalPermutation::LogBaseTwo(N);
            unsigned int butterfly_width;
            unsigned int n_butterfly_groups;
            unsigned int group_start;
            unsigned int butterfly_index;
            unsigned int access_stride = 2 * stride;
            XFloatType* H0;
            XFloatType* H1;
            XFloatType* W;

            for(unsigned int stage = 0; stage < logN; stage++)
            {
                //compute the number of butterfly groups
                n_butterfly_groups = MHO_BitReversalPermutation::TwoToThePowerOf(stage);

                //compute the width of each butterfly
                butterfly_width = N / (2 * n_butterfly_groups);
                for(unsigned int n = 0; n < n_butterfly_groups; n++)
                {
                    //compute the starting index of this butterfly group
                    group_start = 2 * n * butterfly_width;
                    for(unsigned int k = 0; k < butterfly_width; k++)
                    {
                        butterfly_index = group_start + k; //index

                        H0 = data + (access_stride * butterfly_index);
                        H1 = H0 + access_stride * butterfly_width;
                        W = twiddle + 2 * n_butterfly_groups * k;
                        ButterflyRadixTwo_GentlemanSande(H0, H1, W);
                    }
                }
            }
        }

        /**
         * @brief Function ButterflyRadixTwo_GentlemanSande
         * See page 25 of "Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000
         * @param H0 (XFloatType*)
         * @param H1 (XFloatType*)
         * @param W (XFloatType*)
         * @note This is a static function.
         */
        static inline void ButterflyRadixTwo_GentlemanSande(XFloatType* H0, XFloatType* H1, XFloatType* W)
        {
            //H0 is the element from the even indexed array
            //H1 is the element from the odd index array
            //W is the twiddle factor

            //fetch the data
            XFloatType H00, H01, H10, H11, W0, W1, alpha_i, alpha_r, h1_r, h1_i;
            H00 = H0[0];
            H01 = H0[1];
            H10 = H1[0];
            H11 = H1[1];
            W0 = W[0];
            W1 = W[1];

            //cache H1
            h1_r = H10;
            h1_i = H11;

            //compute new h1
            H10 = H00 - h1_r;
            H11 = H01 - h1_i;

            //compute new H0
            H00 += h1_r;
            H01 += h1_i;

            //multiply new h1 by twiddle factor
            alpha_r = W0 * H10 - W1 * H11;
            alpha_i = W1 * H10 + W0 * H11;

            H10 = alpha_r;
            H11 = alpha_i;

            //write out
            H0[0] = H00;
            H0[1] = H01;
            H1[0] = H10;
            H1[1] = H11;
        }

        /**
         * @brief  Radix-2 DIT FFT wrapper for a std::complex array
         *
         * @param N Size of the data array
         * @param data Input/output complex data array with given stride
         * @param twiddle Precomputed twiddle factors array
         * @param stride Stride for accessing data elements
         * @note This is a static function.
         */
        static void FFTRadixTwo_DIT(unsigned int N, std::complex< XFloatType >* data, std::complex< XFloatType >* twiddle,
                                    unsigned int stride = 1)
        {
            FFTRadixTwo_DIT(N, (XFloatType*)&(data[0]), (XFloatType*)&(twiddle[0]), stride);
        }

        /**
         * @brief  Radix-2 DIF FFT wrapper for a std::complex array
         *
         * @param N Size of the data array
         * @param data Input/output complex data array
         * @param twiddle Output twiddle factors array
         * @param stride Stride for accessing data elements
         * @note This is a static function.
         */
        static void FFTRadixTwo_DIF(unsigned int N, std::complex< XFloatType >* data, std::complex< XFloatType >* twiddle,
                                    unsigned int stride = 1)
        {
            FFTRadixTwo_DIF(N, (XFloatType*)&(data[0]), (XFloatType*)&(twiddle[0]), stride);
        }

        ////////////////////////////////////////////////////////////////////////
        //Bluestein/Chirp-Z Algorithm for arbitrary N
        //"Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000

        /**
         * @brief Function ComputeBluesteinArraySize
         * Computes the array size needed to perform a Bluestein/Chirp-Z FFT for arbitrary N
         * @param N (unsigned int)
         * @return Return value (unsigned int)
         * @note This is a static function.
         */
        static unsigned int ComputeBluesteinArraySize(unsigned int N) //returns smallest M = 2^p >= (2N - 2);
        {
            unsigned int M = 2 * (N - 1);
            if(MHO_BitReversalPermutation::IsPowerOfTwo(M))
            {
                return M;
            }
            else
            {
                return MHO_BitReversalPermutation::NextLowestPowerOfTwo(M);
            }
        }

        /**
         * @brief Function ComputeBluesteinScaleFactors
         *
         * @param N (unsigned int)
         * @param scale (std::complex< XFloatType >*) scale factor array must be length N
         * @note This is a static function.
         */
        static void ComputeBluesteinScaleFactors(unsigned int N, std::complex< XFloatType >* scale);

        /**
         * @brief Function ComputeBluesteinCirculantVector
         * twiddle and circulant array must be length M = 2^p >= (2N - 2), where N is the data length
         * @param N (unsigned int)
         * @param M (unsigned int)
         * @param twiddle (std::complex< XFloatType >*)
         * @param scale (std::complex< XFloatType >*)
         * @param circulant (std::complex< XFloatType >*)
         * @note This is a static function.
         */
        static void ComputeBluesteinCirculantVector(unsigned int N, unsigned int M, std::complex< XFloatType >* twiddle,
                                                    std::complex< XFloatType >* scale, std::complex< XFloatType >* circulant)
        {
            //STEP B
            unsigned int mid = M - N + 1;
            for(unsigned int i = 0; i < N; i++)
            {
                //we take the conjugate here because the way we have computed the scale factors
                circulant[i] = std::conj(scale[i]);
            }

            for(unsigned int i = mid; i < M; i++)
            {
                //we take the conjugate here because the way we have computed the scale factors
                circulant[i] = std::conj(scale[M - i]);
            }

            for(unsigned int i = N; i < mid; i++)
            {
                circulant[i] = 0.0;
            }

            //STEP C
            //now we perform an in-place DFT on the circulant vector

            //expects normal order input, produces bit-address permutated output
            MHO_FastFourierTransformUtilities::FFTRadixTwo_DIF(M, circulant, twiddle);
        }

        /**
         * @brief Function Bluestein algorithm for arbitrary length, N is length of the data, (supports strided data access)
         *
         * @param N (unsigned int) length of data
         * @param M (unsigned int) Bluestein array size
         * @param data (std::complex< XFloatType >*) data array
         * @param twiddle (std::complex< XFloatType >*) twiddle factors
         * @param conj_twiddle (std::complex< XFloatType >*) conjugate twiddle factors
         * @param scale (std::complex< XFloatType >*) scale factors
         * @param circulant (std::complex< XFloatType >*) circulant array
         * @param workspace (std::complex< XFloatType >*) scratch space
         * @param stride (unsigned int) (default is 1, unstrided)
         * @note This is a static function.
         */
        static void FFTBluestein(unsigned int N, unsigned int M, std::complex< XFloatType >* data,
                                 std::complex< XFloatType >* twiddle, std::complex< XFloatType >* conj_twiddle,
                                 std::complex< XFloatType >* scale, std::complex< XFloatType >* circulant,
                                 std::complex< XFloatType >* workspace, unsigned int stride = 1)
        {

            //STEP D
            //copy the data into the workspace and scale by the scale factor
            for(unsigned int i = 0; i < N; i++)
            {
                workspace[i] = data[i * stride] * scale[i];
            }

            //fill out the rest of the extended vector with zeros
            for(unsigned int i = N; i < M; i++)
            {
                workspace[i] = std::complex< XFloatType >(0.0, 0.0);
            }

            //STEP E
            //perform the DFT on the workspace
            //do radix-2 FFT w/ decimation in frequency (normal order input, bit-address permutated output)
            FFTRadixTwo_DIF(M, (XFloatType*)(&(workspace[0])), (XFloatType*)(&(twiddle[0])));

            //STEP F
            //now we scale the workspace with the circulant vector
            for(unsigned int i = 0; i < M; i++)
            {
                workspace[i] *= circulant[i];
            }

            //STEP G
            //now perform the inverse DFT on the workspace
            //do radix-2 FFT w/ decimation in time (bit-address permutated input, normal order output)
            FFTRadixTwo_DIT(M, (XFloatType*)(&(workspace[0])), (XFloatType*)(&(conj_twiddle[0])));

            //STEP H
            //renormalize to complete IDFT, extract and scale at the same time
            XFloatType norm = 1.0 / ((XFloatType)M);
            for(unsigned int i = 0; i < N; i++)
            {
                data[i * stride] = norm * workspace[i] * scale[i];
            }
        }

}; //end of class

////////////////////////////////////////////////////////////////////////////////
//template specializations for float, double, and long double

/**
 * @brief Function MHO_FastFourierTransformUtilities<float>::ComputeTwiddleFactors
 *
 * @param N (unsigned int)
 * @param twiddle (std::complex< float >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< float)
 */
template<>
inline void MHO_FastFourierTransformUtilities< float >::ComputeTwiddleFactors(unsigned int N, std::complex< float >* twiddle)
{
    //using std::cos and std::sin is more accurate than the recursive method
    //to compute the twiddle factors
    float di, dN;
    dN = N;
    for(unsigned int i = 0; i < N; i++)
    {
        di = i;
        twiddle[i] = std::complex< float >(std::cos((2.0 * M_PI * di) / dN), std::sin((2.0 * M_PI * di) / dN));
    }
}

/**
 * @brief Function MHO_FastFourierTransformUtilities<double>::ComputeTwiddleFactors
 *
 * @param N (unsigned int)
 * @param twiddle (std::complex< double >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< double)
 */
template<>
inline void MHO_FastFourierTransformUtilities< double >::ComputeTwiddleFactors(unsigned int N, std::complex< double >* twiddle)
{
    //using std::cos and std::sin is more accurate than the recursive method
    //to compute the twiddle factors
    double di, dN;
    dN = N;
    for(unsigned int i = 0; i < N; i++)
    {
        di = i;
        twiddle[i] = std::complex< double >(std::cos((2.0 * M_PI * di) / dN), std::sin((2.0 * M_PI * di) / dN));
    }
}

/**
 * @brief Function MHO_FastFourierTransformUtilities<long double>::ComputeTwiddleFactors
 *
 * @param N (unsigned int)
 * @param twiddle (std::complex< long double >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< long double)
 */
template<>
inline void MHO_FastFourierTransformUtilities< long double >::ComputeTwiddleFactors(unsigned int N,
                                                                                    std::complex< long double >* twiddle)
{
    //using std::cos and std::sin is more accurate than the recursive method
    //to compute the twiddle factors
    long double di, dN;
    dN = N;
    for(unsigned int i = 0; i < N; i++)
    {
        di = i;
        twiddle[i] = std::complex< long double >(cosl((2.0 * M_PIl * di) / dN), sinl((2.0 * M_PIl * di) / dN));
    }
}

////////////////////////////////////////////////////////////////////////////////

//template specializations for float, double, and long double

/**
 * @brief Function MHO_FastFourierTransformUtilities<float>::ComputeTwiddleFactorBasis
 *
 * @param log2N (unsigned int)
 * @param twiddle (std::complex< float >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< float)
 */
template<>
inline void MHO_FastFourierTransformUtilities< float >::ComputeTwiddleFactorBasis(unsigned int log2N,
                                                                                  std::complex< float >* twiddle)
{
    float dN, di;
    dN = MHO_BitReversalPermutation::TwoToThePowerOf(log2N);
    di = 1;
    for(std::size_t i = 0; i < log2N; i++)
    {
        twiddle[i] = std::complex< float >(std::cos((2.0 * M_PI * di) / dN), std::sin((2.0 * M_PI * di) / dN));
        di *= 2;
    }
}

/**
 * @brief Function MHO_FastFourierTransformUtilities<double>::ComputeTwiddleFactorBasis
 *
 * @param log2N (unsigned int)
 * @param twiddle (std::complex< double >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< double)
 */
template<>
inline void MHO_FastFourierTransformUtilities< double >::ComputeTwiddleFactorBasis(unsigned int log2N,
                                                                                   std::complex< double >* twiddle)
{
    double dN, di;
    dN = MHO_BitReversalPermutation::TwoToThePowerOf(log2N);
    di = 1;
    for(std::size_t i = 0; i < log2N; i++)
    {
        twiddle[i] = std::complex< double >(std::cos((2.0 * M_PI * di) / dN), std::sin((2.0 * M_PI * di) / dN));
        di *= 2;
    }
}

/**
 * @brief Function MHO_FastFourierTransformUtilities<long double>::ComputeTwiddleFactorBasis
 *
 * @param log2N (unsigned int)
 * @param twiddle (std::complex< long double >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< long double)
 */
template<>
inline void MHO_FastFourierTransformUtilities< long double >::ComputeTwiddleFactorBasis(unsigned int log2N,
                                                                                        std::complex< long double >* twiddle)
{
    long double dN, di;
    dN = MHO_BitReversalPermutation::TwoToThePowerOf(log2N);
    di = 1;
    for(std::size_t i = 0; i < log2N; i++)
    {
        twiddle[i] = std::complex< long double >(std::cos((2.0 * M_PI * di) / dN), std::sin((2.0 * M_PI * di) / dN));
        di *= 2;
    }
}

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Function MHO_FastFourierTransformUtilities<float>::ComputeBluesteinScaleFactors
 *
 * @param N (unsigned int)
 * @param scale (std::complex< float >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< float)
 * @note
 * IMPORTANT NOTE!
 * This function computes the CONJUGATE of the scale factors as
 * defined by equation 13.22, page 127 of
 * "Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000
 * we do this so we can avoid having to take a complex conjugate of the scale
 * factors when performing the actual FFT
 */
template<>
inline void MHO_FastFourierTransformUtilities< float >::ComputeBluesteinScaleFactors(unsigned int N,
                                                                                     std::complex< float >* scale)
{
    //STEP A
    float theta = M_PI / ((float)N);
    unsigned int i2;
    float x;

    for(unsigned int i = 0; i < N; i++)
    {
        i2 = i * i % (2 * N); //taking the modulus here results in a more accurate DFT/IDFT
        x = theta * i2;
        scale[i] = std::complex< float >(std::cos(x), std::sin(x));
    }
}

/**
 * @brief Function MHO_FastFourierTransformUtilities<double>::ComputeBluesteinScaleFactors
 *
 * @param N (unsigned int)
 * @param scale (std::complex< double >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< double)
 * @note
 * IMPORTANT NOTE!
 * This function computes the CONJUGATE of the scale factors as
 * defined by equation 13.22, page 127 of
 * "Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000
 * we do this so we can avoid having to take a complex conjugate of the scale
 * factors when performing the actual FFT
 */
template<>
inline void MHO_FastFourierTransformUtilities< double >::ComputeBluesteinScaleFactors(unsigned int N,
                                                                                      std::complex< double >* scale)
{
    //STEP A
    double theta = M_PI / ((double)N);
    unsigned int i2;
    double x;

    for(unsigned int i = 0; i < N; i++)
    {
        i2 = i * i % (2 * N); //taking the modulus here results in a more accurate DFT/IDFT
        x = theta * i2;
        scale[i] = std::complex< double >(cos(x), sin(x));
    }
}

/**
 * @brief Function MHO_FastFourierTransformUtilities<long double>::ComputeBluesteinScaleFactors
 *
 * @param N (unsigned int)
 * @param scale (std::complex< long double >*)
 * @return Return value (void MHO_FastFourierTransformUtilities< long double)
 * @note
 * IMPORTANT NOTE!
 * This function computes the CONJUGATE of the scale factors as
 * defined by equation 13.22, page 127 of
 * "Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000
 * we do this so we can avoid having to take a complex conjugate of the scale
 * factors when performing the actual FFT
 */
template<>
inline void MHO_FastFourierTransformUtilities< long double >::ComputeBluesteinScaleFactors(unsigned int N,
                                                                                           std::complex< long double >* scale)
{
    //STEP A
    long double theta = M_PIl / ((long double)N);
    unsigned int i2;
    long double x;

    for(unsigned int i = 0; i < N; i++)
    {
        i2 = i * i % (2 * N); //taking the modulus here results in a more accurate DFT/IDFT
        x = theta * i2;
        scale[i] = std::complex< long double >(cosl(x), sinl(x));
    }
}

} // namespace hops

#endif /*! MHO_FastFourierTransformUtilities_H__ */
