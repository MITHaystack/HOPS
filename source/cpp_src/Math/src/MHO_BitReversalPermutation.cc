#include <bitset>
#include <cstddef>

#include "MHO_BitReversalPermutation.hh"
#include "MHO_Message.hh"

namespace hops
{

bool MHO_BitReversalPermutation::IsPowerOfTwo(unsigned int N)
{
    //taken from Bit Twiddling Hacks
    //http://graphics.stanford.edu/~seander/bithacks.html
    return N && !(N & (N - 1));
}

unsigned int MHO_BitReversalPermutation::LogBaseTwo(unsigned int N)
{
    //taken from Bit Twiddling Hacks
    //http://graphics.stanford.edu/~seander/bithacks.html
    unsigned int p = 0;
    while(N >>= 1)
    {
        p++;
    }
    return p;
}

unsigned int MHO_BitReversalPermutation::TwoToThePowerOf(unsigned int N)
{
    unsigned int val = 1;
    return (val << N);
}

unsigned int MHO_BitReversalPermutation::NextLowestPowerOfTwo(unsigned int N)
{
    if(IsPowerOfTwo(N))
    {
        return N;
    }
    else
    {
        unsigned int p = LogBaseTwo(N);
        return TwoToThePowerOf(p + 1);
    }
}

//takes the input value x, and the number of bits, and returns
//the number which is composed of the bits of x in reverse order
unsigned int MHO_BitReversalPermutation::ReverseIndexBits(unsigned int nbits, unsigned int x)
{
    unsigned int val = 0;
    for(unsigned int i = 0; i < nbits; i++)
    {
        val |= ((1 & ((x & (1 << i)) >> i)) << ((nbits - 1) - i));
    }
    return val;
}

bool MHO_BitReversalPermutation::IsPowerOfBase(unsigned int N, unsigned int B)
{
    //check if N is a perfect power of B, this is very slow!!
    if(N < B)
    {
        return false;
    }
    else
    {
        unsigned int i = 1;
        while(i < N)
        {
            i *= B;
        }
        if(N == i)
        {
            return true;
        }
        return false;
    }
}

unsigned int MHO_BitReversalPermutation::RaiseBaseToThePower(unsigned int B, unsigned int N)
{
    unsigned int val = 1;
    for(unsigned int i = 0; i < N; i++)
    {
        val *= B;
    }
    return val;
}

unsigned int MHO_BitReversalPermutation::LogBaseB(unsigned int N, unsigned int B)
{
    //we assume that N is a perfect power of B
    //but if not we return the leading power
    if(N != 0)
    {
        if(N == 1)
        {
            return 0;
        }
        unsigned int power = 0;
        unsigned int quotient = N;
        do
        {
            quotient /= B;
            power++;
        }
        while(quotient > 1);
        return power;
    }
    else
    {
        //error
        return 0;
    }
}

void MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(unsigned int N, unsigned int* index_arr)
{
    //this function uses the recursive Buneman algorithm to compute
    //the bit reversed permutation of an array of length N = 2^p with entries 0,1,2...N-1
    //the permutated indices are stored in index_arr
    //this is slow but simple
    //for details see
    //Fast Fourier Transforms by James S. Walker, CRC Press
    if(IsPowerOfTwo(N) && N != 0)
    {
        unsigned int p = LogBaseTwo(N);

        if(N == 1) //p = 0
        {
            index_arr[0] = 0;
            return;
        }

        if(N == 2) //p = 1
        {
            index_arr[0] = 0;
            index_arr[1] = 1;
            return;
        }

        index_arr[0] = 0;
        index_arr[1] = 1;

        unsigned int mid;
        for(unsigned int r = 2; r <= p; r++)
        {
            mid = TwoToThePowerOf(r - 1);
            for(unsigned int q = 0; q < mid; q++)
            {
                index_arr[q] *= 2;
                index_arr[q + mid] = index_arr[q] + 1;
            }
        }
    }
}

} // namespace hops
