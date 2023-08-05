#ifndef MHO_FastFourierUtilities_Defined_H
#define MHO_FastFourierUtilities_Defined_H

#include "hopsocl_defines.h"
#include "MHO_ComplexUtils.cl"
#include "MHO_BitReversalPermutation.cl"


////////////////////////////////////////////////////////////////////////
//RADIX-2
void FFTRadixTwo_DITStrided(unsigned int N, unsigned int stride, __constant const CL_TYPE2* twiddle, CL_TYPE2* data)
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
                W = twiddle[n_butterfly_groups*k];
                
                //here we use the Cooly-Tukey butterfly
                //multiply H1 by twiddle factor to get W*H1, store temporary workspace Z
                Z = ComplexMultiply(H1, W);
                //compute the update
                //H0' = H0 + W*H1
                //H1' = H0 - W*H1
                H1 = H0;
                H0 += Z;
                H1 -= Z;
                data[x] = H0;
                data[y] = H1;
            }
        }
    }
}


//RADIX-2 DIF
void 
FFTRadixTwo_DIFStrided(unsigned int N, unsigned int stride, __constant const CL_TYPE2* twiddle, CL_TYPE2* data)
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
                W = twiddle[n_butterfly_groups*k];

                //here we use Gentleman Sande butterfly
                Z = H1; //first cache H1 in Z
                H1 = H0 - Z; //set H1' = H0 - H1
                H0 += Z; //set H0 = H0 + H1
                //multiply H1 by twiddle factor to get W*H1, to obtain H1' = (H0 - H1)*W
                Z = ComplexMultiply(H1,W);

                data[stride*butterfly_index] = H0;
                data[stride*(butterfly_index+butterfly_width)]=Z;
            }
        }
    }
}



////////////////////////////////////////////////////////////////////////
//RADIX-2
void FFTRadixTwo_DIT(unsigned int N, __constant const CL_TYPE2* twiddle, CL_TYPE2* data)
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
                x = butterfly_index;
                y = (butterfly_index + butterfly_width);
            
                H0 = data[x];
                H1 = data[y];
                W = twiddle[n_butterfly_groups*k];
                
                //here we use the Cooly-Tukey butterfly
                //multiply H1 by twiddle factor to get W*H1, store temporary workspace Z
                Z = ComplexMultiply(H1, W);
                //compute the update
                //H0' = H0 + W*H1
                //H1' = H0 - W*H1
                H1 = H0;
                H0 += Z;
                H1 -= Z;
                data[x] = H0;
                data[y] = H1;
            }
        }
    }
}


//RADIX-2 DIF
void 
FFTRadixTwo_DIF(unsigned int N, __constant const CL_TYPE2* twiddle, CL_TYPE2* data)
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
    unsigned int x,y;
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
                x = butterfly_index;
                y = (butterfly_index + butterfly_width);
                H0 = data[x];
                H1 = data[y];
                W = twiddle[n_butterfly_groups*k];

                //here we use Gentleman Sande butterfly
                Z = H1; //first cache H1 in Z
                H1 = H0 - Z; //set H1' = H0 - H1
                H0 += Z; //set H0 = H0 + H1
                //multiply H1 by twiddle factor to get W*H1, to obtain H1' = (H0 - H1)*W
                Z = ComplexMultiply(H1,W);

                data[x] = H0;
                data[y]=Z;
            }
        }
    }
}































// 
// 
// FFTBluestein(unsigned int N,
//              unsigned int M,
//              __global CL_TYPE2* twiddle, //must be size M
//              __global CL_TYPE2* conj_twiddle, //must be size M
//              __global CL_TYPE2* scale, //must be size N
//              __global CL_TYPE2* circulant, //must be size M
//              CL_TYPE2* data) //must be size M, initially filled up to N
// 
// 
// 
// 
// //Bluestein algorithm for arbitrary length, N is length of the data, strided data access
// void FFTBluestein(unsigned int N,
//                   unsigned int M,
//                   unsigned int stride,
//                   CL_TYPE2* data,
//                   __global CL_TYPE2* twiddle,
//                   __global CL_TYPE2* conj_twiddle,
//                   __global CL_TYPE2* scale,
//                   __global CL_TYPE2* circulant,
//                   __global CL_TYPE2* workspace)
// {
//     //STEP D
//     //copy the data into the workspace and scale by the scale factor
//     for(unsigned int i=0; i<N; i++)
//     {
//         workspace[i] = ComplexMultiply(data[i*stride], scale[i]);
//     }
// 
//     //fill out the rest of the extended vector with zeros
//     CL_TYPE2 Z = 0.0;
//     for(unsigned int i=N; i<M; i++)
//     {
//         workspace[i] = Z;
//     }
// 
//     //STEP E
//     //perform the DFT on the workspace
//     //do radix-2 FFT w/ decimation in frequency (normal order input, bit-address permutated output)
//     FFTRadixTwo_DIF(M, 1, workspace, twiddle );
// 
//     //STEP F
//     //now we scale the workspace with the circulant vector
//     for(unsigned int i=0; i<M; i++)
//     {
//         workspace[i] = ComplexMultiply(workspace[i], circulant[i]);
//     }
// 
//     //STEP G
//     //now perform the inverse DFT on the workspace
//     //do radix-2 FFT w/ decimation in time (bit-address permutated input, normal order output)
//     FFTRadixTwo_DIT(M, 1, workspace, conj_twiddle);
// 
//     //STEP H
//     //renormalize to complete IDFT, extract and scale at the same time
//     CL_TYPE norm = 1.0/((CL_TYPE)M);
//     for(unsigned int i=0; i<N; i++)
//     {
//         data[i*stride] = norm*ComplexMultiply(workspace[i], scale[i]);
//     }
// }
// 
// 
// 
// 
// 
// ////////////////////////////////////////////////////////////////////////////////
// //Bluestein Algorithm
// 
// void
// #ifdef FFT_USE_CONST_MEM
// FFTBluestein(unsigned int N,
//              unsigned int M,
//              __constant CL_TYPE2* twiddle, //must be size M
//              __constant CL_TYPE2* conj_twiddle, //must be size M
//              __constant CL_TYPE2* scale, //must be size N
//              __constant CL_TYPE2* circulant, //must be size M
//              CL_TYPE2* data) //must be size M, initially filled up to N
// #else
// FFTBluestein(unsigned int N,
//              unsigned int M,
//              __global CL_TYPE2* twiddle, //must be size M
//              __global CL_TYPE2* conj_twiddle, //must be size M
//              __global CL_TYPE2* scale, //must be size N
//              __global CL_TYPE2* circulant, //must be size M
//              CL_TYPE2* data) //must be size M, initially filled up to N
// #endif
// {
// 
// 
//     //STEP D
//     //copy the data into the workspace and scale by the scale factor
//     CL_TYPE2 B;
//     CL_TYPE2 A;
//     CL_TYPE2 Z;
// 
//     for(size_t i=0; i<N; i++)
//     {
//         A = data[i];
//         B = scale[i];
//         Z.s0 = (A.s0)*(B.s0) - (A.s1)*(B.s1);
//         Z.s1 =  (A.s1)*(B.s0) + (A.s0)*(B.s1);
//         data[i] = Z;
//     }
// 
//     //fill out the rest of the extended vector with zeros
//     Z = 0.0;
//     for(size_t i=N; i<M; i++)
//     {
//         data[i] = Z;
//     }
// 
//     //do a decimation in frequency radix-2 FFT
//     FFTRadixTwo_DIF(M, data, twiddle);
// 
//     //STEP F
//     //now we scale the workspace with the circulant vector, and conjugate for input to dft
//     for(size_t i=0; i<M; i++)
//     {
//         A = data[i];
//         B = circulant[i];
//         Z.s0 = (A.s0)*(B.s0) - (A.s1)*(B.s1);
//         Z.s1 = ( (A.s0)*(B.s1) + (A.s1)*(B.s0) );
//         data[i] = Z;
//     }
// 
//     //do a decimation in time radix-2 inverse FFT
//     FFTRadixTwo_DIT(M, data, conj_twiddle);
// 
//     //STEP H
//     //renormalize to complete IDFT, extract and scale at the same time
//     CL_TYPE norm = 1.0/((CL_TYPE)M);
//     for(size_t i=0; i<N; i++)
//     {
//         A = data[i];
//         B = scale[i];
// 
//         Z.s0 = (A.s0)*(B.s0) - (A.s1)*(B.s1);
//         Z.s1 = (A.s0)*(B.s1) + (A.s1)*(B.s0);
//         data[i] = norm*Z;
//     }
// }



#endif /* MHO_FastFourierUtilities_Defined_H */
