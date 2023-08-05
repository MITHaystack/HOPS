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


void 
PermuteArrayNoBranch(unsigned int N, unsigned int stride, __global const unsigned int* permutation_index_arr, __global CL_TYPE2* arr)
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
PermuteArray(unsigned int N, unsigned int stride, __global const unsigned int* permutation_index_arr, __global CL_TYPE2* arr)
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
            // val = arr[x];
            // arr[x] = arr[y];
            // arr[y] = val;
        }
    }
}





#endif /* MHO_BitReversalPermutation_CL_H__ */
