#ifndef MHO_FastFourierTransformUtilities_HH__
#define MHO_FastFourierTransformUtilities_HH__

#include <complex>
#include <cstddef>
#include <cmath>

#include "MHO_Message.hh"
#include "MHO_BitReversalPermutation.hh"

namespace hops
{


template< typename XFloatType>
class MHO_FastFourierTransformUtilities
{
    public:
        MHO_FastFourierTransformUtilities(){};
        virtual ~MHO_FastFourierTransformUtilities(){};
        
        //conjugate an array
        static void Conjugate(unsigned int N, std::complex< XFloatType >* array)
        {
            for(unsigned int i=0; i<N; i++){ array[i] = std::conj(array[i]); }
        }

        //strided, conjugate array
        static void Conjugate(unsigned int N, std::complex< XFloatType >* array, unsigned int stride)
        {
            for(unsigned int i=0; i<N; i++){ array[i*stride] = std::conj(array[i*stride]); }
        }

        ////////////////////////////////////////////////////////////////////////
        //compute all the twiddle factors e^{i*2*pi/N} for 0 to N-1
        static void ComputeTwiddleFactors(unsigned int N, std::complex< XFloatType >* twiddle);

        static void ComputeConjugateTwiddleFactors(unsigned int N, std::complex< XFloatType >* conj_twiddle)
        {
            //using std::cos and std::sin is more accurate than the recursive method
            //to compute the twiddle factors
            ComputeTwiddleFactors(N, conj_twiddle);
            Conjugate(N, conj_twiddle);
        }
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        //RADIX-2
        static void FFTRadixTwo_DIT(unsigned int N, XFloatType* data, XFloatType* twiddle)
        {
            //decimation in time

            //input: data array in bit-address permutated order
            //output: fft of data in normal order
            if(MHO_BitReversalPermutation::IsPowerOfTwo(N) )
            {
                unsigned int logN = MHO_BitReversalPermutation::LogBaseTwo(N);

                unsigned int butterfly_width;
                unsigned int n_butterfly_groups;
                unsigned int group_start;
                unsigned int butterfly_index;

                for(unsigned int stage = 0; stage < logN; stage++)
                {
                    //compute the width of each butterfly
                    butterfly_width = MHO_BitReversalPermutation::TwoToThePowerOf(stage);

                    //compute the number of butterfly groups
                    n_butterfly_groups = N/(2*butterfly_width);

                    for(unsigned int n = 0; n < n_butterfly_groups; n++)
                    {
                        //compute the starting index of this butterfly group
                        group_start = 2*n*butterfly_width;

                        for(unsigned int k=0; k < butterfly_width; k++)
                        {
                            butterfly_index = group_start + k; //index

                            ButterflyRadixTwo_CooleyTukey( &(data[2*butterfly_index]),
                                                           &(data[2*butterfly_index + 2*butterfly_width]),
                                                           &(twiddle[2*n_butterfly_groups*k]) );
                        }
                    }

                }
            }
            else
            {
                msg_error("math", "MHO_FastFourierTransformUtilities::FFTRadixTwo_DIT: error, array has length: "<<N<<" which is not an integer power of 2."<<eom);
            }
        }
        
        //RADIX-2, STRIDED data access overload
        static void FFTRadixTwo_DIT(unsigned int N, XFloatType* data, XFloatType* twiddle, unsigned int stride)
        {
            //decimation in time

            //input: data array in bit-address permutated order
            //output: fft of data in normal order

            if(MHO_BitReversalPermutation::IsPowerOfTwo(N) )
            {
                unsigned int logN = MHO_BitReversalPermutation::LogBaseTwo(N);

                unsigned int butterfly_width;
                unsigned int n_butterfly_groups;
                unsigned int group_start;
                unsigned int butterfly_index;

                for(unsigned int stage = 0; stage < logN; stage++)
                {
                    //compute the width of each butterfly
                    butterfly_width = MHO_BitReversalPermutation::TwoToThePowerOf(stage);

                    //compute the number of butterfly groups
                    n_butterfly_groups = N/(2*butterfly_width);

                    for(unsigned int n = 0; n < n_butterfly_groups; n++)
                    {
                        //compute the starting index of this butterfly group
                        group_start = 2*n*butterfly_width;

                        for(unsigned int k=0; k < butterfly_width; k++)
                        {
                            butterfly_index = group_start + k; //index

                            ButterflyRadixTwo_CooleyTukey( &(data[2*butterfly_index*stride]),
                                                           &(data[2*butterfly_index*stride + 2*butterfly_width*stride]),
                                                           &(twiddle[2*n_butterfly_groups*k]) );
                        }
                    }

                }
            }
            else
            {
                msg_error("math", "MHO_FastFourierTransformUtilities::FFTRadixTwo_DIT: error, array has length: "<<N<<" which is not an integer power of 2."<<eom);
            }
        }
        


        static void ButterflyRadixTwo_CooleyTukey(XFloatType* H0, XFloatType* H1, XFloatType* W)
        {
            ////////////////////////////////////////////////////////////////////////
            //See page 23
            //"Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000

            //H0 is the element from the even indexed array
            //H1 is the element from the odd index array
            //W is the twiddle factor

            //multiply H1 by the twiddle factor to get W*H1 and store in H1
            XFloatType alpha_r = W[0]*H1[0] - W[1]*H1[1];
            XFloatType alpha_i = W[1]*H1[0] + W[0]*H1[1];

            H1[0] = H0[0] - alpha_r;
            H1[1] = H0[1] - alpha_i;

            H0[0] = H0[0] + alpha_r;
            H0[1] = H0[1] + alpha_i;
        }

        //RADIX-2 DIF
        static void FFTRadixTwo_DIF(unsigned int N, XFloatType* data, XFloatType* twiddle)
        {
            //decimation in frequency

            //input: data array in normal order
            //output: fft of data in bit-address permutated order

            if(MHO_BitReversalPermutation::IsPowerOfTwo(N) )
            {
                unsigned int logN = MHO_BitReversalPermutation::LogBaseTwo(N);

                unsigned int butterfly_width;
                unsigned int n_butterfly_groups;
                unsigned int group_start;
                unsigned int butterfly_index;

                for(unsigned int stage = 0; stage < logN; stage++)
                {
                    //compute the number of butterfly groups
                    n_butterfly_groups= MHO_BitReversalPermutation::TwoToThePowerOf(stage);

                    //compute the width of each butterfly
                    butterfly_width =  N/(2*n_butterfly_groups);

                    for(unsigned int n = 0; n < n_butterfly_groups; n++)
                    {
                        //compute the starting index of this butterfly group
                        group_start = 2*n*butterfly_width;

                        for(unsigned int k=0; k < butterfly_width; k++)
                        {
                            butterfly_index = group_start + k; //index

                            ButterflyRadixTwo_GentlemanSande( &(data[2*butterfly_index]),
                                                              &(data[2*butterfly_index + 2*butterfly_width]),
                                                              &(twiddle[2*n_butterfly_groups*k]) );
                        }
                    }

                }
            }
            else
            {
                msg_error("math", "MHO_FastFourierTransformUtilities::FFTRadixTwo_DIF: error, array has length: "<<N<<" which is not an integer power of 2."<< eom);
            }

        }
        
        
        //RADIX-2 DIF, strided data access
        static void FFTRadixTwo_DIF(unsigned int N, XFloatType* data, XFloatType* twiddle, unsigned int stride)
        {
            //decimation in frequency

            //input: data array in normal order
            //output: fft of data in bit-address permutated order

            if(MHO_BitReversalPermutation::IsPowerOfTwo(N) )
            {
                unsigned int logN = MHO_BitReversalPermutation::LogBaseTwo(N);

                unsigned int butterfly_width;
                unsigned int n_butterfly_groups;
                unsigned int group_start;
                unsigned int butterfly_index;

                for(unsigned int stage = 0; stage < logN; stage++)
                {
                    //compute the number of butterfly groups
                    n_butterfly_groups= MHO_BitReversalPermutation::TwoToThePowerOf(stage);

                    //compute the width of each butterfly
                    butterfly_width =  N/(2*n_butterfly_groups);

                    for(unsigned int n = 0; n < n_butterfly_groups; n++)
                    {
                        //compute the starting index of this butterfly group
                        group_start = 2*n*butterfly_width;

                        for(unsigned int k=0; k < butterfly_width; k++)
                        {
                            butterfly_index = group_start + k; //index

                            ButterflyRadixTwo_GentlemanSande( &(data[2*butterfly_index*stride]),
                                                              &(data[2*butterfly_index*stride + 2*butterfly_width*stride]),
                                                              &(twiddle[2*n_butterfly_groups*k]) );
                        }
                    }

                }
            }
            else
            {
                msg_error("math", "MHO_FastFourierTransformUtilities::FFTRadixTwo_DIF: error, array has length: "<<N<<" which is not an integer power of 2."<< eom);
            }

        }


        static void ButterflyRadixTwo_GentlemanSande(XFloatType* H0, XFloatType* H1, XFloatType* W)
        {
            ////////////////////////////////////////////////////////////////////////
            //See page 25
            //"Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000

            //H0 is the element from the even indexed array
            //H1 is the element from the odd index array
            //W is the twiddle factor

            //cache H1
            XFloatType h1_r = H1[0];
            XFloatType h1_i = H1[1];

            //compute new h1
            H1[0] = H0[0] - h1_r;
            H1[1] = H0[1] - h1_i;

            //compute new n0
            H0[0] += h1_r;
            H0[1] += h1_i;

            //multiply new h1 by twiddle factor
            XFloatType alpha_r = W[0]*H1[0] - W[1]*H1[1];
            XFloatType alpha_i = W[1]*H1[0] + W[0]*H1[1];

            H1[0] = alpha_r;
            H1[1] = alpha_i;
        }

        //wrappers for complex array
        static void FFTRadixTwo_DIT(unsigned int N, std::complex< XFloatType >* data, std::complex< XFloatType >* twiddle)
        {
            FFTRadixTwo_DIT(N, (XFloatType*)&(data[0]), (XFloatType*) &(twiddle[0]) );
        }

        static void FFTRadixTwo_DIF(unsigned int N, std::complex< XFloatType >* data, std::complex< XFloatType >* twiddle)
        {
            FFTRadixTwo_DIF(N, (XFloatType*)&(data[0]), (XFloatType*) &(twiddle[0]) );
        }

        //wrappers for complex array, strided
        static void FFTRadixTwo_DIT(unsigned int N, std::complex< XFloatType >* data, std::complex< XFloatType >* twiddle, unsigned int stride)
        {
            FFTRadixTwo_DIT(N, (XFloatType*)&(data[0]), (XFloatType*) &(twiddle[0]), stride);
        }

        static void FFTRadixTwo_DIF(unsigned int N, std::complex< XFloatType >* data, std::complex< XFloatType >* twiddle, unsigned int stride)
        {
            FFTRadixTwo_DIF(N, (XFloatType*)&(data[0]), (XFloatType*) &(twiddle[0]), stride);
        }


        ////////////////////////////////////////////////////////////////////////
        //Bluestein/Chirp-Z Algorithm for arbitrary N
        //"Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000

        static unsigned int ComputeBluesteinArraySize(unsigned int N) //returns smallest M = 2^p >= (2N - 2);
        {
            unsigned int M = 2*(N - 1);
            if( MHO_BitReversalPermutation::IsPowerOfTwo(M))
            {
                return M;
            }
            else
            {
                return MHO_BitReversalPermutation::NextLowestPowerOfTwo(M);
            }
        }


        //scale factor array must be length N
        static void ComputeBluesteinScaleFactors(unsigned int N, std::complex< XFloatType >* scale);

        //twiddle and circulant array must be length M = 2^p >= (2N - 2), where N is the data length
        static void ComputeBluesteinCirculantVector(unsigned int N,
                                                    unsigned int M,
                                                    std::complex< XFloatType >* twiddle,
                                                    std::complex< XFloatType >* scale,
                                                    std::complex< XFloatType >* circulant)
        {
            //STEP B
            unsigned int mid = M - N + 1;

            for(unsigned int i=0; i<N; i++)
            {
                //we take the conjugate here because the way we have computed the scale factors
                circulant[i] = std::conj(scale[i]);
            }

            for(unsigned int i=mid; i<M; i++)
            {
                //we take the conjugate here because the way we have computed the scale factors
                circulant[i] = std::conj(scale[M-i]);
            }

            for(unsigned int i=N; i<mid; i++)
            {
                circulant[i] = 0.0;
            }

            //STEP C
            //now we perform an in-place DFT on the circulant vector

            //expects normal order input, produces bit-address permutated output
            MHO_FastFourierTransformUtilities::FFTRadixTwo_DIF(M, circulant, twiddle);
        }


        //Bluestein algorithm for arbitrary length, N is length of the data
        static void FFTBluestein(unsigned int N,
                                 unsigned int M,
                                 std::complex< XFloatType >* data,
                                 std::complex< XFloatType >* twiddle,
                                 std::complex< XFloatType >* conj_twiddle,
                                 std::complex< XFloatType >* scale,
                                 std::complex< XFloatType >* circulant,
                                 std::complex< XFloatType >* workspace)
        {

            //STEP D
            //copy the data into the workspace and scale by the scale factor
            for(unsigned int i=0; i<N; i++)
            {
                workspace[i] = data[i]*scale[i];
            }

            //fill out the rest of the extended vector with zeros
            for(unsigned int i=N; i<M; i++)
            {
                workspace[i] = std::complex<XFloatType>(0.0,0.0);
            }

            //STEP E
            //perform the DFT on the workspace
            //do radix-2 FFT w/ decimation in frequency (normal order input, bit-address permutated output)
            FFTRadixTwo_DIF(M, (XFloatType*)(&(workspace[0])), (XFloatType*)(&(twiddle[0])) );

            //STEP F
            //now we scale the workspace with the circulant vector
            for(unsigned int i=0; i<M; i++)
            {
                workspace[i] *= circulant[i];
            }

            //STEP G
            //now perform the inverse DFT on the workspace
            //do radix-2 FFT w/ decimation in time (bit-address permutated input, normal order output)
            FFTRadixTwo_DIT(M, (XFloatType*)(&(workspace[0])), (XFloatType*)(&(conj_twiddle[0])) );

            //STEP H
            //renormalize to complete IDFT, extract and scale at the same time
            XFloatType norm = 1.0/((XFloatType)M);
            for(unsigned int i=0; i<N; i++)
            {
                data[i] = norm*workspace[i]*scale[i];
            }
        }





        //Bluestein algorithm for arbitrary length, N is length of the data, strided data access overload
        static void FFTBluestein(unsigned int N,
                                 unsigned int M,
                                 std::complex< XFloatType >* data,
                                 std::complex< XFloatType >* twiddle,
                                 std::complex< XFloatType >* conj_twiddle,
                                 std::complex< XFloatType >* scale,
                                 std::complex< XFloatType >* circulant,
                                 std::complex< XFloatType >* workspace,
                                 unsigned int stride)
        {

            //STEP D
            //copy the data into the workspace and scale by the scale factor
            for(unsigned int i=0; i<N; i++)
            {
                workspace[i] = data[i*stride]*scale[i];
            }

            //fill out the rest of the extended vector with zeros
            for(unsigned int i=N; i<M; i++)
            {
                workspace[i] = std::complex<XFloatType>(0.0,0.0);
            }

            //STEP E
            //perform the DFT on the workspace
            //do radix-2 FFT w/ decimation in frequency (normal order input, bit-address permutated output)
            FFTRadixTwo_DIF(M, (XFloatType*)(&(workspace[0])), (XFloatType*)(&(twiddle[0])) );

            //STEP F
            //now we scale the workspace with the circulant vector
            for(unsigned int i=0; i<M; i++)
            {
                workspace[i] *= circulant[i];
            }

            //STEP G
            //now perform the inverse DFT on the workspace
            //do radix-2 FFT w/ decimation in time (bit-address permutated input, normal order output)
            FFTRadixTwo_DIT(M, (XFloatType*)(&(workspace[0])), (XFloatType*)(&(conj_twiddle[0])) );

            //STEP H
            //renormalize to complete IDFT, extract and scale at the same time
            XFloatType norm = 1.0/((XFloatType)M);
            for(unsigned int i=0; i<N; i++)
            {
                data[i*stride] = norm*workspace[i]*scale[i];
            }
        }
        
}; //end of class



////////////////////////////////////////////////////////////////////////////////
//template specializations for float, double, and long double

template<>
inline void
MHO_FastFourierTransformUtilities<float>::ComputeTwiddleFactors(unsigned int N, std::complex< float >* twiddle)
{
    //using std::cos and std::sin is more accurate than the recursive method
    //to compute the twiddle factors
    float di, dN;
    dN = N;
    for(unsigned int i=0; i<N; i++)
    {
        di = i;
        twiddle[i] = std::complex<float>(std::cos((2.0*M_PI*di)/dN), std::sin((2.0*M_PI*di)/dN));
    }
}


template<>
inline void
MHO_FastFourierTransformUtilities<double>::ComputeTwiddleFactors(unsigned int N, std::complex< double >* twiddle)
{
    //using std::cos and std::sin is more accurate than the recursive method
    //to compute the twiddle factors
    double di, dN;
    dN = N;
    for(unsigned int i=0; i<N; i++)
    {
        di = i;
        twiddle[i] = std::complex<double>(std::cos((2.0*M_PI*di)/dN), std::sin((2.0*M_PI*di)/dN));
    }
}


template<>
inline void
MHO_FastFourierTransformUtilities<long double>::ComputeTwiddleFactors(unsigned int N, std::complex< long double >* twiddle)
{
    //using std::cos and std::sin is more accurate than the recursive method
    //to compute the twiddle factors
    long double di, dN;
    dN = N;
    for(unsigned int i=0; i<N; i++)
    {
        di = i;
        twiddle[i] = std::complex<long double>(cosl((2.0*M_PIl*di)/dN), sinl((2.0*M_PIl*di)/dN));
    }
}


template<>
inline void
MHO_FastFourierTransformUtilities<float>::ComputeBluesteinScaleFactors(unsigned int N, std::complex< float >* scale)
{
    //STEP A
    float theta = M_PI/((float)N);
    unsigned int i2;
    float x;

    for(unsigned int i=0; i<N; i++)
    {
        i2 = i*i % (2*N); //taking the modulus here results in a more accurate DFT/IDFT
        x = theta*i2;
        scale[i] = std::complex<float>( std::cos(x), std::sin(x) );
    }

    //IMPORTANT NOTE!
    //This function computes the CONJUGATE of the scale factors as
    //defined by equation 13.22, page 127 of
    //"Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000
    //we do this so we can avoid having to take a complex conjugate of the scale
    //factors when performing the actual FFT
}

template<>
inline void
MHO_FastFourierTransformUtilities<double>::ComputeBluesteinScaleFactors(unsigned int N, std::complex< double >* scale)
{
    //STEP A
    double theta = M_PI/((double)N);
    unsigned int i2;
    double x;

    for(unsigned int i=0; i<N; i++)
    {
        i2 = i*i % (2*N); //taking the modulus here results in a more accurate DFT/IDFT
        x = theta*i2;
        scale[i] = std::complex<double>( cosl(x), sin(x) );
    }

    //IMPORTANT NOTE!
    //This function computes the CONJUGATE of the scale factors as
    //defined by equation 13.22, page 127 of
    //"Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000
    //we do this so we can avoid having to take a complex conjugate of the scale
    //factors when performing the actual FFT
}

template<>
inline void
MHO_FastFourierTransformUtilities<long double>::ComputeBluesteinScaleFactors(unsigned int N, std::complex< long double >* scale)
{
    //STEP A
    long double theta = M_PIl/((long double)N);
    unsigned int i2;
    long double x;

    for(unsigned int i=0; i<N; i++)
    {
        i2 = i*i % (2*N); //taking the modulus here results in a more accurate DFT/IDFT
        x = theta*i2;
        scale[i] = std::complex<long double>( cosl(x), sinl(x) );
    }

    //IMPORTANT NOTE!
    //This function computes the CONJUGATE of the scale factors as
    //defined by equation 13.22, page 127 of
    //"Inside the FFT Black Box", E. Chu and A. George, Ch. 13, CRC Press, 2000
    //we do this so we can avoid having to take a complex conjugate of the scale
    //factors when performing the actual FFT
}


}


#endif /* MHO_FastFourierTransformUtilities_H__ */
