#ifndef MHO_BitReversalPermutation_CL_H__
#define MHO_BitReversalPermutation_CL_H__

#include "hopsocl_defines.h"


bool
IsPowerOfTwo(unsigned int N)
{
    return N && !(N & (N - 1));
}

unsigned int
LogBaseTwo(unsigned int N)
{
    unsigned int p = 0;
    while (N >>= 1){p++;}
    return p;
}


unsigned int
TwoToThePowerOf(unsigned int N)
{
    unsigned int val = 1;
    return (val << N);
}

unsigned int 
ReverseIndexBits(unsigned int nbits, unsigned int x)
{
    unsigned int val = 0;
    for (unsigned int i = 0; i < nbits; i++)
    {
        val |= ( ( 1 & ((x & (1 << i)) >> i) ) << ( (nbits - 1) - i) );
    }
    return val;
}


void 
PermuteArrayNoBranch(unsigned int N, unsigned int stride, const unsigned int* permutation_index_arr, __global CL_TYPE2* arr)
{
    CL_TYPE2 a,b;
    //expects an array of size N
    unsigned int perm;
    unsigned int x,y;
    int do_swap, sgn;

    for(unsigned int i=0; i<N; i++)
    {
        //branch free way of doing a conditional swap only if (i<perm)
        perm = permutation_index_arr[i];
        x = i*stride;
        y = perm*stride;
        do_swap = (i < perm);
        sgn = (i < perm) - (i >= perm);
        a = arr[x];
        b = arr[y];

        a = a + do_swap*b;
        b = do_swap*a - sgn*b;
        a = a - do_swap*b;
    
        arr[x] = a;
        arr[y] = b;
    }
}



void 
PermuteArrayStridedCached(unsigned int N, unsigned int stride, const unsigned int* permutation_index_arr, CL_TYPE2* arr)
{
    CL_TYPE2 a,b;
    unsigned int x,y;
    for(unsigned int i=0; i<N; i++)
    {
        unsigned int perm = permutation_index_arr[i];
        if(i < perm )
        {
            //swap values
            x = i*stride;
            y = perm*stride;
            a = arr[x];
            b = arr[y];
            arr[x] = b;
            arr[y] = a;
        }
    }
}


void 
PermuteArrayCached(unsigned int N, const unsigned int* permutation_index_arr, CL_TYPE2* arr)
{
    CL_TYPE2 a,b;
    for(unsigned int i=0; i<N; i++)
    {
        unsigned int perm = permutation_index_arr[i];
        if(i < perm )
        {
            //swap values
            a = arr[i];
            b = arr[perm];
            arr[i] = b;
            arr[perm] = a;
        }
    }
}

void 
PermuteArrayStrided(unsigned int N, unsigned int stride, CL_TYPE2* arr)
{
    unsigned int log2N = LogBaseTwo(N);
    unsigned int x,y;
    CL_TYPE2 a,b;
    for(unsigned int i=0; i<N; i++)
    {
        unsigned int perm = ReverseIndexBits(log2N,i);
        if(i < perm )
        {
            //swap values
            x = i*stride;
            y = perm*stride;
            a = arr[x];
            b = arr[y];
            arr[x] = b;
            arr[y] = a;
        }
    }
}



#endif /* MHO_BitReversalPermutation_CL_H__ */
