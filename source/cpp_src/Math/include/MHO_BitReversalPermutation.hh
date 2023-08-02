#ifndef MHO_BitReversalPermutation_HH__
#define MHO_BitReversalPermutation_HH__

#include <cstddef>
#include <vector>

namespace hops
{

class MHO_BitReversalPermutation
{
    public:
        MHO_BitReversalPermutation(){};
        virtual ~MHO_BitReversalPermutation(){};

        static bool IsPowerOfTwo(unsigned int N);
        static unsigned int TwoToThePowerOf(unsigned int N); //N must be >= 0 and <=31
        static unsigned int LogBaseTwo(unsigned int N);
        static unsigned int NextLowestPowerOfTwo(unsigned int N);

        //factor the integer N into powers of the factors listed in factors
        static bool Factor(unsigned int N, unsigned int n_factors, unsigned int* factors, unsigned int* powers);

        static bool IsPowerOfBase(unsigned int N, unsigned int B);
        static unsigned int RaiseBaseToThePower(unsigned int B, unsigned int N); //N must be >= 0
        static unsigned int LogBaseB(unsigned int N, unsigned int B);

        //must have N = 2^P, with P an integer
        static void ComputeBitReversedIndicesBaseTwo(unsigned int N, unsigned int* index_arr);

        //must have length N = B^P, with P an integer
        //B is the base of the number system used to compute the bit indices
        static void ComputeBitReversedIndices(unsigned int N, unsigned int B, unsigned int* index_arr);


        //data type of size 1
        template<typename DataType >
        static void PermuteArray(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr, unsigned int stride = 1)
        {
            //expects an array of size N
            DataType val;
            for(unsigned int i=0; i<N; i++)
            {
                unsigned int perm = permutation_index_arr[i];
                if(i < perm )
                {
                    //swap values
                    val = arr[i*stride];
                    arr[i*stride] = arr[ perm*stride ];
                    arr[perm*stride] = val;
                }
            }
        }



    private:
};


}


#endif /* MHO_BitReversalPermutation_H__ */
