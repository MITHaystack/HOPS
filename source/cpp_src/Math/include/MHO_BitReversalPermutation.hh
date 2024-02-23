#ifndef MHO_BitReversalPermutation_HH__
#define MHO_BitReversalPermutation_HH__

#include <cstddef>

#define USE_STDALGO_SWAP

#ifdef USE_STDALGO_SWAP
#include <algorithm>
#endif

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
        static unsigned int ReverseIndexBits(unsigned int nbits, unsigned int x);

        static bool IsPowerOfBase(unsigned int N, unsigned int B);
        static unsigned int RaiseBaseToThePower(unsigned int B, unsigned int N); //N must be >= 0
        static unsigned int LogBaseB(unsigned int N, unsigned int B);

        //must have N = 2^P, with P an integer
        static void ComputeBitReversedIndicesBaseTwo(unsigned int N, unsigned int* index_arr);

        //non-strided data access pattern
        template<typename DataType >
        static void PermuteArray(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr)
        {
            //expects an array of size N
            DataType val;
            for(unsigned int i=0; i<N; i++)
            {
                unsigned int perm = permutation_index_arr[i];
                if(i < perm )
                {
                    //swap values
                    #ifdef USE_STDALGO_SWAP
                    std::swap(arr[i], arr[perm]);
                    #else
                    val = arr[i];
                    arr[i] = arr[ perm];
                    arr[perm] = val;
                    #endif
                }
            }
        }

        //strided data access version
        template<typename DataType >
        static void PermuteArray(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr, unsigned int stride)
        {
            //expects an array of size N
            DataType val;
            for(unsigned int i=0; i<N; i++)
            {
                unsigned int perm = permutation_index_arr[i];
                if(i < perm )
                {
                    //swap values
                    #ifdef USE_STDALGO_SWAP
                    std::swap(arr[i*stride], arr[perm*stride]);
                    #else
                    val = arr[i*stride];
                    arr[i*stride] = arr[ perm*stride ];
                    arr[perm*stride] = val;
                    #endif
                }
            }
        }



        //non-strided data access pattern
        //branch free (this is actually slower on CPU, but we preserve it here for comparison as this method is used on GPU)
        template<typename DataType >
        static void PermuteArrayBranchFree(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr)
        {
            DataType a,b;
            //expects an array of size N
            unsigned int perm;
            typename DataType::value_type do_swap;
            typename DataType::value_type sgn;
            for(unsigned int i=0; i<N; i++)
            {
                perm = permutation_index_arr[i];
                do_swap = (i < perm);
                sgn = (i < perm) - (i >= perm);
                a = arr[i];
                b = arr[perm];

                a = a + do_swap*b;
                b = do_swap*a - sgn*b;
                a = a - do_swap*b;

                arr[i] = a;
                arr[perm] = b;
            }
        }

        //strided data access version
        //branch free (this is actually slower on CPU, but we preserve it here for comparison as this method is used on GPU)
        template<typename DataType >
        static void PermuteArrayBranchFree(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr, unsigned int stride)
        {
            DataType a,b;
            //expects an array of size N
            unsigned int perm;
            typename DataType::value_type do_swap;
            typename DataType::value_type sgn;
            for(unsigned int i=0; i<N; i++)
            {

                perm = permutation_index_arr[i];
                do_swap = (i < perm);
                sgn = (i < perm) - (i >= perm);
                a = arr[i*stride];
                b = arr[perm*stride];

                a = a + do_swap*b;
                b = do_swap*a - sgn*b;
                a = a - do_swap*b;

                arr[i*stride] = a;
                arr[perm*stride] = b;
            }
        }

    private:
};

}


#endif /*! MHO_BitReversalPermutation_H__ */
