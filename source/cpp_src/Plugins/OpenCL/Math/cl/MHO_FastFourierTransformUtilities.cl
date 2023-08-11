#ifndef MHO_FastFourierUtilities_Defined_H
#define MHO_FastFourierUtilities_Defined_H

#include "hopsocl_defines.h"
#include "MHO_ComplexUtils.cl"
#include "MHO_BitReversalPermutation.cl"

#define FFT_FORWARD -1.0
#define FFT_BACKWARD 1.0

#define NORMAL_TWIDDLE 1.0
#define CONJ_TWIDDLE -1.0


unsigned int ComputeBluesteinArraySize(unsigned int N) 
{
    //returns smallest M = 2^p >= (2N - 2);
    unsigned int M = 2*(N - 1);
    if(IsPowerOfTwo(M)){return M;}
    else{ return NextLowestPowerOfTwo(M);}
}


void ComputeTwiddleFactorBasis(unsigned int log2N, __local CL_TYPE2* twiddle)
{
    double dN, di, arg;
    dN = TwoToThePowerOf(log2N);
    di = 1;
    for(unsigned int i=0; i<log2N;i++)
    {
        arg = (2.0*M_PI*di)/dN;
        twiddle[i].s0 = cos(arg);
        twiddle[i].s1 = sin(arg);
        di *= 2;
    }
}

//conjugate_flag must be 1 (no conjugate) or -1 (conjugate)
CL_TYPE2 ComputeTwiddleFactor(unsigned int log2N, unsigned int index, __local const CL_TYPE2* basis, CL_TYPE conj_flag)
{
    unsigned int bit = 1;
    CL_TYPE2 val;
    val.s0 = 1.0; 
    val.s1 = 0.0;
    for(unsigned int i=0; i<log2N; i++)
    {
        if( (index & bit) ){val = ComplexMultiply(val,basis[i]);}
        bit *=2;
    }
    val.s1 *= conj_flag;
    return val;
}

////////////////////////////////////////////////////////////////////////////////
//calculates the number of 1d FFTs, and shape of work items
unsigned int CalculateWorkItemInfo(unsigned int NDIM, //total number of dimensions
                                   unsigned int D, //selected dimension
                                   unsigned int* dim, //size of array in each dimension 
                                   unsigned int* non_active_dimension_index, //index selected in each non-active dimension
                                   unsigned int* non_active_dimension_size) //sizes of each non-active dimension
{
    //figure out the total number of 1-D FFTs to perform along this (active) axis
    unsigned int n_fft = 1;
    unsigned int count = 0;
    for(unsigned int i = 0; i < NDIM; i++)
    {
        if(i != D)
        {
            n_fft *= dim[i];
            non_active_dimension_index[count] = i;
            non_active_dimension_size[count] = dim[i];
            count++;
        }
    }
    //returns the total number of FFTs that need to be performed
    return n_fft;
}

////////////////////////////////////////////////////////////////////////////////
//RADIX2 DIT with strided data locations.
//The twiddle factors are computed on-the-fly from a factor-basis.
void FFTRadixTwo_DIT(unsigned int N, 
                     unsigned int stride, 
                     CL_TYPE conj_flag, //conjugate the twiddle factors
                     CL_TYPE direction, //-1 = forward, 1 = backward
                     __local const CL_TYPE2* twiddle_basis, 
                     __global CL_TYPE2* data)
{
    //temporary workspace
    CL_TYPE2 H0;
    CL_TYPE2 H1;
    CL_TYPE2 W;
    CL_TYPE2 Z;

    unsigned int logN = LogBaseTwo(N);
    unsigned int butterfly_width;
    unsigned int n_butterfly_groups;
    unsigned int group_start;
    unsigned int butterfly_index;
    unsigned int x,y;

    for(unsigned int stage = 0; stage < logN; stage++)
    {
        //compute the width of each butterfly
        butterfly_width = TwoToThePowerOf(stage);
        //compute the number of butterfly groups
        n_butterfly_groups = N/(2*butterfly_width);
        for(unsigned int n = 0; n < n_butterfly_groups; n++)
        {
            //compute the starting index of this butterfly group
            group_start = 2*n*butterfly_width;
            for(unsigned int k=0; k < butterfly_width; k++)
            {
                butterfly_index = group_start + k; //index
                x = stride*butterfly_index;
                y = stride*(butterfly_index + butterfly_width);
            
                H0 = data[x];
                H1 = data[y];
                W = ComputeTwiddleFactor(logN, n_butterfly_groups*k, twiddle_basis, conj_flag);

                //conjugate if the direction flag is -1 (but only on the first pass)
                if(stage == 0)
                {
                    H0.s1 *= direction;
                    H1.s1 *= direction;
                }

                //here we use the Cooly-Tukey butterfly
                //multiply H1 by twiddle factor to get W*H1, store temporary workspace Z
                Z = ComplexMultiply(H1, W);
                //compute the update
                //H0' = H0 + W*H1
                //H1' = H0 - W*H1
                H1 = H0;
                H0 += Z;
                H1 -= Z;

                //conjugate if the direction flag is -1 (but only on the last pass)
                if(stage == logN-1)
                {
                    H0.s1 *= direction;
                    H1.s1 *= direction;
                }

                data[x] = H0;
                data[y] = H1;
            }
        }
    }
}



//RADIX-2 DIF
void 
FFTRadixTwo_DIF(unsigned int N, 
                unsigned int stride, 
                CL_TYPE conj_flag, 
                CL_TYPE direction, //-1 = forward, 1 = backward
                __local const CL_TYPE2* twiddle_basis,
                CL_TYPE2* data)
{
    //temporary workspace
    CL_TYPE2 H0;
    CL_TYPE2 H1;
    CL_TYPE2 W;
    CL_TYPE2 Z;
    
    //decimation in frequency, N is assumed to be a power of 2
    unsigned int logN = LogBaseTwo(N);
    unsigned int butterfly_width;
    unsigned int n_butterfly_groups;
    unsigned int group_start;
    unsigned int butterfly_index;
    for(unsigned int stage = 0; stage < logN; stage++)
    {
        //compute the number of butterfly groups
        n_butterfly_groups= TwoToThePowerOf(stage);
    
        //compute the width of each butterfly
        butterfly_width =  N/(2*n_butterfly_groups);
        for(unsigned int n = 0; n < n_butterfly_groups; n++)
        {
            //compute the starting index of this butterfly group
            group_start = 2*n*butterfly_width;
            for(unsigned int k=0; k < butterfly_width; k++)
            {
                butterfly_index = group_start + k; //index
                H0 = data[stride*butterfly_index];
                H1 = data[stride*(butterfly_index+butterfly_width)];
                //conjugate if the direction flag is -1 (but only on the first pass)
                if(stage == 0)
                {
                    H0.s1 *= direction;
                    H1.s1 *= direction;
                }

                W = ComputeTwiddleFactor(logN, n_butterfly_groups*k, twiddle_basis, conj_flag);

                //here we use Gentleman Sande butterfly
                Z = H1; //first cache H1 in Z
                H1 = H0 - Z; //set H1' = H0 - H1
                H0 += Z; //set H0 = H0 + H1
                //multiply H1 by twiddle factor to get W*H1, to obtain H1' = (H0 - H1)*W
                Z = ComplexMultiply(H1,W);

                //conjugate if the direction flag is -1 (but only on the last pass)
                if(stage == logN-1)
                {
                    H0.s1 *= direction;
                    Z.s1 *= direction;
                }

                data[stride*butterfly_index] = H0;
                data[stride*(butterfly_index+butterfly_width)]=Z;
            }
        }
    }
}


//Bluestein algorithm for arbitrary length, N is length of the data, strided data access
void 
#ifdef FFT_USE_CONST_MEM
FFTBluestein(unsigned int N,
             unsigned int M,
             unsigned int stride,
             __local const CL_TYPE2* twiddle_basis,
             __global CL_TYPE2* data,
             __constant CL_TYPE2* scale,
             __constant CL_TYPE2* circulant,
             __global CL_TYPE2* workspace)
#else
FFTBluestein(unsigned int N,
             unsigned int M,
             unsigned int stride,
             __local const CL_TYPE2* twiddle_basis,
             __global CL_TYPE2* data,
             __global CL_TYPE2* scale,
             __global CL_TYPE2* circulant,
             __global CL_TYPE2* workspace)
#endif
{
    //STEP D
    //copy the data into the workspace and scale by the scale factor
    for(unsigned int i=0; i<N; i++){workspace[i] = ComplexMultiply(data[i*stride], scale[i]);}
    //fill out the rest of the extended vector with zeros
    for(unsigned int i=N; i<M; i++){workspace[i] = 0.0;}

    //STEP E
    //perform the DFT on the workspace
    //do radix-2 FFT w/ decimation in frequency (normal order input, bit-address permutated output)
    FFTRadixTwo_DIF(M, 1, NORMAL_TWIDDLE, FFT_FORWARD, twiddle_basis, workspace); //stride=1, using normal twiddle factors

    //STEP F
    //now we scale the workspace with the circulant vector
    for(unsigned int i=0; i<M; i++){workspace[i] = ComplexMultiply(workspace[i], circulant[i]);}

    //STEP G
    //now perform the inverse DFT on the workspace
    //do radix-2 FFT w/ decimation in time (bit-address permutated input, normal order output)
    FFTRadixTwo_DIT(M, 1, CONJ_TWIDDLE, FFT_FORWARD, twiddle_basis, workspace); //stride=1, use conjugate twiddle factors

    //STEP H
    //renormalize to complete IDFT, extract and scale at the same time
    CL_TYPE norm = 1.0/((CL_TYPE)M);
    for(unsigned int i=0; i<N; i++){data[i*stride] = norm*ComplexMultiply(workspace[i], scale[i]);}

}


#endif /* MHO_FastFourierUtilities_Defined_H */
