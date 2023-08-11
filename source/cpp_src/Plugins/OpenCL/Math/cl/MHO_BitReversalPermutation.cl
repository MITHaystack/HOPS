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
NextLowestPowerOfTwo(unsigned int N)
{
    if(IsPowerOfTwo(N) ){return N;}
    else
    {
        unsigned int p = LogBaseTwo(N);
        return TwoToThePowerOf(p+1);
    }
}

unsigned int 
ReverseIndexBits(unsigned int nbits, unsigned int x)
{
    unsigned int val = 0;
    for (unsigned int i = 0; i < nbits; i++)
    {
        //this is just reversing the order of the bits of x between 0 and nbits
        val |= ( ( 1 & ((x & (1 << i)) >> i) ) << ( (nbits - 1) - i) );
    }
    return val;
}

void 
PermuteArray(unsigned int N, unsigned int stride, __global CL_TYPE2* arr)
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
