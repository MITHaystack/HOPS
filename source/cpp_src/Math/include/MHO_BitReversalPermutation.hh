#ifndef MHO_BitReversalPermutation_HH__
#define MHO_BitReversalPermutation_HH__

#include <cstddef>

#define USE_STDALGO_SWAP

#ifdef USE_STDALGO_SWAP
    #include <algorithm>
#endif

namespace hops
{

/*!
 *@file MHO_BitReversalPermutation.hh
 *@class MHO_BitReversalPermutation
 *@date Fri Oct 23 12:02:01 2020 -0400
 *@brief bit reversal permutation function for power-of-two FFTs
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_BitReversalPermutation
{
    public:
        MHO_BitReversalPermutation(){};
        virtual ~MHO_BitReversalPermutation(){};

        /**
         * @brief Checks if an unsigned integer is a power of two.
         * 
         * @param N Input unsigned integer to check.
         * @return True if N is a power of two, false otherwise.
         * @note This is a static function.
         */
        static bool IsPowerOfTwo(unsigned int N);
        /**
         * @brief Calculates 2 raised to the power of N using bit shifting.
         * 
         * @param N Input exponent for the calculation.
         * @return Result of 2^N as an unsigned integer.
         * @note This is a static function.
         */
        static unsigned int TwoToThePowerOf(unsigned int N); //N must be >= 0 and <=31
        /**
         * @brief Calculates the logarithm base two of an unsigned integer N using bitwise operations.
         * 
         * @param N Input unsigned integer for which to calculate the logarithm base two.
         * @return The logarithm base two of N as an unsigned integer.
         * @note This is a static function.
         */
        static unsigned int LogBaseTwo(unsigned int N);
        /**
         * @brief Calculates the next lowest power of two for a given unsigned integer.
         * 
         * @param N Input unsigned integer.
         * @return Next lowest power of two as an unsigned integer.
         * @note This is a static function.
         */
        static unsigned int NextLowestPowerOfTwo(unsigned int N);
        /**
         * @brief Reverses the bit indices of a given unsigned integer.
         * 
         * @param nbits Number of bits to consider for reversal.
         * @param x Input unsigned integer whose bit indices are to be reversed.
         * @return The unsigned integer with its bit indices reversed.
         * @note This is a static function.
         */
        static unsigned int ReverseIndexBits(unsigned int nbits, unsigned int x);

        /**
         * @brief Checks if an unsigned integer N is a perfect power of another unsigned integer B.
         * 
         * @param N Input number to check for being a perfect power
         * @param B Base number against which N is checked
         * @return True if N is a perfect power of B, false otherwise
         * @note This is a static function.
         */
        static bool IsPowerOfBase(unsigned int N, unsigned int B);
        /**
         * @brief Calculates B raised to the power N.
         * 
         * @param B Base number to be raised
         * @param N Power to which base is raised
         * @return Result of B^N
         * @note This is a static function.
         */
        static unsigned int RaiseBaseToThePower(unsigned int B, unsigned int N); //N must be >= 0
        /**
         * @brief Calculates the logarithm base B of N, assuming N is a perfect power of B.
         * 
         * @param N Input number for which to calculate the logarithm
         * @param B Base of the logarithm
         * @return Leading power of B that divides N
         * @note This is a static function.
         */
        static unsigned int LogBaseB(unsigned int N, unsigned int B);

        //must have N = 2^P, with P an integer
        /**
         * @brief Computes bit-reversed indices using Buneman algorithm for input size N = 2^P.
         * 
         * @param N Input size, must be a power of two
         * @param index_arr Output array to store permutated indices
         * @note This is a static function.
         */
        static void ComputeBitReversedIndicesBaseTwo(unsigned int N, unsigned int* index_arr);

        //non-strided data access pattern
        /**
         * @brief Permutes an array using a given permutation index array.
         * 
         * @tparam DataType Template parameter DataType
         * @param N Size of the array and permutation index array.
         * @param permutation_index_arr Array containing permutation indices.
         * @param arr (DataType*)
         * @note This is a static function.
         */
        template< typename DataType >
        static void PermuteArray(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr)
        {
            //expects an array of size N
            DataType val;
            for(unsigned int i = 0; i < N; i++)
            {
                unsigned int perm = permutation_index_arr[i];
                if(i < perm)
                {
//swap values
#ifdef USE_STDALGO_SWAP
                    std::swap(arr[i], arr[perm]);
#else
                    val = arr[i];
                    arr[i] = arr[perm];
                    arr[perm] = val;
#endif
                }
            }
        }

        //strided data access version
        /**
         * @brief Permutes a DataType array using an index permutation.
         * 
         * @tparam DataType Template parameter DataType
         * @param N Size of the array and permutation index array.
         * @param permutation_index_arr Array containing the permutation indices.
         * @param arr DataType array to be permuted.
         * @param stride (unsigned int)
         * @note This is a static function.
         */
        template< typename DataType >
        static void PermuteArray(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr, unsigned int stride)
        {
            //expects an array of size N
            DataType val;
            for(unsigned int i = 0; i < N; i++)
            {
                unsigned int perm = permutation_index_arr[i];
                if(i < perm)
                {
//swap values
#ifdef USE_STDALGO_SWAP
                    std::swap(arr[i * stride], arr[perm * stride]);
#else
                    val = arr[i * stride];
                    arr[i * stride] = arr[perm * stride];
                    arr[perm * stride] = val;
#endif
                }
            }
        }

        //non-strided data access pattern
        //branch free (this is actually slower on CPU, but we preserve it here for comparison as this method is used on GPU)
        /**
         * @brief Function PermuteArrayBranchFree
         * 
         * @tparam DataType Template parameter DataType
         * @param N (unsigned int)
         * @param permutation_index_arr (const unsigned int*)
         * @param arr (DataType*)
         * @note This is a static function.
         */
        template< typename DataType >
        static void PermuteArrayBranchFree(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr)
        {
            DataType a, b;
            //expects an array of size N
            unsigned int perm;
            typename DataType::value_type do_swap;
            typename DataType::value_type sgn;
            for(unsigned int i = 0; i < N; i++)
            {
                perm = permutation_index_arr[i];
                do_swap = (i < perm);
                sgn = (i < perm) - (i >= perm);
                a = arr[i];
                b = arr[perm];

                a = a + do_swap * b;
                b = do_swap * a - sgn * b;
                a = a - do_swap * b;

                arr[i] = a;
                arr[perm] = b;
            }
        }

        //strided data access version
        //branch free (this is actually slower on CPU, but we preserve it here for comparison as this method is used on GPU)
        /**
         * @brief Function PermuteArrayBranchFree
         * 
         * @tparam DataType Template parameter DataType
         * @param N (unsigned int)
         * @param permutation_index_arr (const unsigned int*)
         * @param arr (DataType*)
         * @param stride (unsigned int)
         * @note This is a static function.
         */
        template< typename DataType >
        static void PermuteArrayBranchFree(unsigned int N, const unsigned int* permutation_index_arr, DataType* arr,
                                           unsigned int stride)
        {
            DataType a, b;
            //expects an array of size N
            unsigned int perm;
            typename DataType::value_type do_swap;
            typename DataType::value_type sgn;
            for(unsigned int i = 0; i < N; i++)
            {

                perm = permutation_index_arr[i];
                do_swap = (i < perm);
                sgn = (i < perm) - (i >= perm);
                a = arr[i * stride];
                b = arr[perm * stride];

                a = a + do_swap * b;
                b = do_swap * a - sgn * b;
                a = a - do_swap * b;

                arr[i * stride] = a;
                arr[perm * stride] = b;
            }
        }

    private:
};

} // namespace hops

#endif /*! MHO_BitReversalPermutation_H__ */
