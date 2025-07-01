#ifndef MHO_NDArrayMath_HH__
#define MHO_NDArrayMath_HH__

#include <cmath>
#include <cstddef>

namespace hops
{

/*!
 *@file MHO_NDArrayMath.hh
 *@class MHO_NDArrayMath
 *@date Sun Jan 24 14:53:01 2021 -0500
 *@brief utility functions for multidimensional array access
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_NDArrayMath
{
    public:
        MHO_NDArrayMath(){};
        virtual ~MHO_NDArrayMath(){};

        // //modulus of two integers
        // static std::size_t Modulus(int arg, int n)
        // {
        //     //returns positive arg mod n, may want to optimize this for speed
        //     double div = ( (double)arg )/( (double) n);
        //     return (std::size_t)(std::fabs( (double)arg - std::floor(div)*((double)n) ) );
        // }

        //modulus of two integers
        /**
         * @brief Calculates the modulus of two integers.
         * 
         * @param arg First integer operand
         * @param n Second integer operand (modulus)
         * @return Result of arg modulo n
         * @note This is a static function.
         */
        static std::size_t Modulus(std::size_t arg, std::size_t n) { return arg % n; }

        //for a multidimensional array (using row major indexing) which has the
        //dimensions specified in DimSize, this function computes the offset from
        //the first element given the indices in the array Index
        /**
         * @brief Calculates offset in a multidimensional array using row-major indexing.
         * 
         * @tparam RANK Template parameter RANK
         * @param DimSize Pointer to an array containing dimension sizes.
         * @param Index Pointer to an array of indices.
         * @return Offset from the first element given the input indices.
         * @note This is a static function.
         */
        template< std::size_t RANK >
        inline static std::size_t OffsetFromRowMajorIndex(const std::size_t* DimSize, const std::size_t* Index)
        {
            std::size_t val = Index[0];
            for(std::size_t i = 1; i < RANK; i++)
            {
                val *= DimSize[i];
                val += Index[i];
            }
            return val;
        }

        //for a multidimensional array (using row major indexing) which has the
        //strides specified in Strides, this function computes the offset from
        //the first element given the indices in the array Index
        /**
         * @brief Calculates offset for a given index in a multidimensional array using row-major indexing.
         * 
         * @tparam RANK Template parameter RANK
         * @param Strides Array of strides for each dimension
         * @param Index Indices for each dimension
         * @return Offset from the first element given the indices
         * @note This is a static function.
         */
        template< std::size_t RANK >
        inline static std::size_t OffsetFromStrideIndex(const std::size_t* Strides, const std::size_t* Index)
        {
            std::size_t val = 0;
            for(std::size_t i = 0; i < RANK; i++)
            {
                val += Index[i] * Strides[i];
            }
            return val;
        }

        //for a multidimensional array (using row major indexing) which has the
        //dimensions specified in DimSize, this function computes the stride between
        //consecutive elements in the selected dimension given that the other indices are fixed
        //the first element given the indices in the array Index
        /**
         * @brief Calculates stride for a given dimension in row-major indexed multidimensional array.
         * 
         * @tparam RANK Template parameter RANK
         * @param selected_dim Selected dimension index
         * @param DimSize Array containing dimensions sizes
         * @return Stride value as std::size_t
         * @note This is a static function.
         */
        template< std::size_t RANK >
        inline static std::size_t StrideFromRowMajorIndex(std::size_t selected_dim, const std::size_t* DimSize)
        {
            std::size_t val = 1;
            for(std::size_t i = 0; i < RANK; i++)
            {
                if(i > selected_dim)
                {
                    val *= DimSize[i];
                };
            }
            return val;
        }

        //for a multidimensional array (using row major indexing) which has the
        //dimensions specified in DimSize, this function computes the indices of
        //the elements which has the given offset from the first element
        /**
         * @brief Function RowMajorIndexFromOffset
         * 
         * @tparam RANK Template parameter RANK
         * @param offset (std::size_t)
         * @param DimSize (const std::size_t*)
         * @param Index (std::size_t*)
         * @note This is a static function.
         */
        template< std::size_t RANK >
        inline static void RowMajorIndexFromOffset(std::size_t offset, const std::size_t* DimSize, std::size_t* Index)
        {
            std::size_t div[RANK];

            //in row major format the last index varies the fastest
            std::size_t i;
            for(std::size_t d = 0; d < RANK; d++)
            {
                i = RANK - d - 1;

                if(d == 0)
                {
                    Index[i] = MHO_NDArrayMath::Modulus(offset, DimSize[i]);
                    div[i] = (offset - Index[i]) / DimSize[i];
                }
                else
                {
                    Index[i] = MHO_NDArrayMath::Modulus(div[i + 1], DimSize[i]);
                    div[i] = (div[i + 1] - Index[i]) / DimSize[i];
                }
            }
        }

        //checks if all the indices in Index are in the valid range
        /**
         * @brief Checks if all indices in Index are within valid range for a multidimensional array.
         * 
         * @param DimSize Pointer to an array containing the dimensions of the multidimensional array.
         * @param Index Pointer to an array containing the indices of the element being accessed.
         * @return Boolean indicating whether all indices are valid (true if valid, false otherwise).
         * @note This is a static function.
         */
        template< std::size_t RANK > inline static bool CheckIndexValidity(const std::size_t* DimSize, const std::size_t* Index)
        {
            for(std::size_t i = 0; i < RANK; i++)
            {
                if(Index[i] >= DimSize[i])
                {
                    return false;
                };
            }
            return true;
        };

        //given the dimensions of an array, computes its total size, assuming all dimensions are non-zero
        /**
         * @brief Calculates total size of an array given its dimensions.
         * 
         * @param DimSize Pointer to an array of dimension sizes.
         * @return Total size of the array as a std::size_t.
         * @note This is a static function.
         */
        template< std::size_t RANK > inline static std::size_t TotalArraySize(const std::size_t* DimSize)
        {
            std::size_t val = 1;
            for(std::size_t i = 0; i < RANK; i++)
            {
                val *= DimSize[i];
            }
            return val;
        }

        //compute 2^N at compile time
        /**
         * @brief Class PowerOfTwo
         */
        template< std::size_t N > struct PowerOfTwo
        {
                enum
                {
                    value = 2 * PowerOfTwo< N - 1 >::value
                };
        };

        //compute integer division at compile time
        /**
         * @brief Class Divide
         */
        template< int numerator, int denominator > struct Divide
        {
                enum
                {
                    value = Divide< numerator, 1 >::value / Divide< denominator, 1 >::value
                };
        };

        /**
         * @brief Calculates reversed indices offsets for given dimensions.
         * 
         * @tparam RANK Template parameter RANK
         * @param DimSize Input array of dimension sizes
         * @param ReversedIndex Output array to store reversed index offsets
         * @note This is a static function.
         */
        template< std::size_t RANK >
        static void OffsetsForReversedIndices(const std::size_t* DimSize, std::size_t* ReversedIndex)
        {
            std::size_t total_array_size = MHO_NDArrayMath::TotalArraySize< RANK >(DimSize);
            std::size_t index[RANK];
            for(std::size_t i = 0; i < total_array_size; i++)
            {
                MHO_NDArrayMath::RowMajorIndexFromOffset< RANK >(i, DimSize, index);
                for(std::size_t j = 0; j < RANK; j++)
                {
                    index[j] = (DimSize[j] - index[j]) % DimSize[j];
                };
                ReversedIndex[i] = MHO_NDArrayMath::OffsetFromRowMajorIndex< RANK >(DimSize, index);
            }
        }

        //increment the multi-dimensional indices by 1
        /**
         * @brief Increment multi-dimensional indices by one in row-major order.
         * 
         * @param DimSize Pointer to an array containing dimension sizes
         * @param Index Pointer to an array containing current indices
         * @return True if increment was successful, false if overflow occurred
         * @note This is a static function.
         */
        template< std::size_t RANK > static bool IncrementIndices(const std::size_t* DimSize, std::size_t* Index)
        {
            for(std::size_t i = 1; i <= RANK; i++)
            {
                if(++Index[RANK - i] < DimSize[RANK - i])
                {
                    return true;
                }
                else
                {
                    Index[RANK - i] = 0;
                }
            }
            //if we have reached here we have overflowed the 0-th dimension
            return false;
        }

        //increment the multi-dimensional indices by the amount in diff
        /**
         * @brief Increment multi-dimensional indices and return true if successful, false otherwise.
         * 
         * @tparam RANK Template parameter RANK
         * @param DimSize Pointer to an array containing dimension sizes
         * @param Index Pointer to an array of indices to increment
         * @param diff (std::size_t)
         * @return Boolean indicating success (true) or failure (false)
         * @note This is a static function.
         */
        template< std::size_t RANK >
        static bool IncrementIndices(const std::size_t* DimSize, std::size_t* Index, std::size_t diff)
        {
            std::size_t offset = OffsetFromRowMajorIndex< RANK >(DimSize, Index);
            offset += diff;
            if(offset < TotalArraySize< RANK >(DimSize))
            {
                RowMajorIndexFromOffset< RANK >(offset, DimSize, Index);
                return true;
            }
            //cannot increment past the end
            return false;
        }

        /**
         * @brief Decrements indices in a multidimensional array using row major indexing.
         * 
         * @param DimSize Pointer to an array containing the dimensions of the multidimensional array.
         * @param Index Pointer to an array containing the current indices.
         * @return True if successful decrement, false if underflowed the 0-th dimension.
         * @note This is a static function.
         */
        template< std::size_t RANK > static bool DecrementIndices(const std::size_t* DimSize, std::size_t* Index)
        {
            for(std::size_t i = 1; i <= RANK; i++)
            {
                if(Index[RANK - i] > 0)
                {
                    --Index[RANK - i];
                    return true;
                }
                else
                {
                    Index[RANK - i] = DimSize[RANK - i] - 1;
                }
            }
            //if we reach here we have underflowed the 0-th dimension
            return false;
        }

        /**
         * @brief Decrements indices in a multidimensional array until an underflow is reached.
         * 
         * @tparam RANK Template parameter RANK
         * @param DimSize Pointer to an array containing the dimensions of the multidimensional array.
         * @param Index Pointer to an array containing the current indices of the multidimensional array.
         * @param diff (std::size_t)
         * @return True if decrementing was successful, false if all dimensions have underflowed.
         * @note This is a static function.
         */
        template< std::size_t RANK >
        static bool DecrementIndices(const std::size_t* DimSize, std::size_t* Index, std::size_t diff)
        {
            std::size_t offset = OffsetFromRowMajorIndex< RANK >(DimSize, Index);
            if(diff < offset)
            {
                offset -= diff;
                RowMajorIndexFromOffset< RANK >(offset, DimSize, Index);
                return true;
            }
            //cannot decrement more than the offset from start
            return false;
        }
};

//specialization for base case of power of two
/**
 * @brief Class MHO_NDArrayMath::PowerOfTwo<0>
 */
template<> struct MHO_NDArrayMath::PowerOfTwo< 0 >
{
        enum
        {
            value = 1
        };
};

//specialization for base case of divide
/**
 * @brief Class MHO_NDArrayMath::Divide<numerator, 1>
 */
template< int numerator > struct MHO_NDArrayMath::Divide< numerator, 1 >
{
        enum
        {
            value = numerator
        };
};

} // namespace hops

#endif /*! MHO_NDArrayMath_HH__ */
