#ifndef HArrayMath_HH__
#define HArrayMath_HH__

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <bitset>
#include <limits.h>
#include <iostream>

namespace hops{


/**
*
*@file HArrayMath.hh
*@class HArrayMath
*@brief collection of math functions used by array library
*@details
*
*
*<b>Revision History:<b>
*Date Name Brief Description
*Tue May 28 21:59:09 EDT 2013 J. Barrett (barrettj@mit.edu) First Version
*
*/

#if defined(__SIZEOF_INT__) && defined(CHAR_BIT)
#define MAX_MORTON_BITS __SIZEOF_INT__*CHAR_BIT
#else
#define MAX_MORTON_BITS 32
#endif

class HArrayMath
{
    public:
        HArrayMath(){};
        virtual ~HArrayMath(){};

        //modulus of two integers
        static std::size_t Modulus(int arg, int n)
        {
            //returns arg mod n;
            double div = ( (double)arg )/( (double) n);
            return (std::size_t)(std::fabs( (double)arg - std::floor(div)*((double)n) ) );
        }

        //for a multidimensional array (using row major indexing) which has the
        //dimensions specified in DimSize, this function computes the offset from
        //the first element given the indices in the array Index
        template<std::size_t NDIM> inline static std::size_t
        OffsetFromRowMajorIndex(const std::size_t* DimSize, const std::size_t* Index)
        {
            std::size_t val = Index[0];
            for(std::size_t i=1; i<NDIM; i++)
            {
                val *= DimSize[i];
                val += Index[i];
            }
            return val;
        }

        //for a multidimensional array (using row major indexing) which has the
        //dimensions specified in DimSize, this function computes the stride between
        //consecutive elements in the selected dimension given that the other indices are fixed
        //the first element given the indices in the array Index
        template<std::size_t NDIM> inline static std::size_t
        StrideFromRowMajorIndex(std::size_t selected_dim, const std::size_t* DimSize)
        {
            std::size_t val = 1;
            for(std::size_t i=0; i<NDIM; i++)
            {
                if(i > selected_dim){val *= DimSize[i];};
            }
            return val;
        }



        //for a multidimensional array (using row major indexing) which has the
        //dimensions specified in DimSize, this function computes the indices of
        //the elements which has the given offset from the first element
        template<std::size_t NDIM> inline static void
        RowMajorIndexFromOffset(std::size_t offset, const std::size_t* DimSize, std::size_t* Index)
        {
            std::size_t div[NDIM];

            //in row major format the last index varies the fastest
            std::size_t i;
            for(std::size_t d=0; d < NDIM; d++)
            {
                i = NDIM - d -1;

                if(d == 0)
                {
                    Index[i] = HArrayMath::Modulus(offset, DimSize[i]);
                    div[i] = (offset - Index[i])/DimSize[i];
                }
                else
                {
                    Index[i] = HArrayMath::Modulus(div[i+1], DimSize[i]);
                    div[i] = (div[i+1] - Index[i])/DimSize[i];
                }
            }
        }

        //checks if all the indices in Index are in the valid range
        template<std::size_t NDIM> inline static bool
        CheckIndexValidity(const std::size_t* DimSize, const std::size_t* Index)
        {
            for(std::size_t i=0; i<NDIM; i++)
            {
                if(Index[i] >= DimSize[i]){return false;};
            }
            return true;
        };


        //given the dimensions of an array, computes its total size, assuming all dimensions are non-zero
        template<std::size_t NDIM> inline static std::size_t
        TotalArraySize(const std::size_t* DimSize)
        {
            std::size_t val = 1;
            for(std::size_t i=0; i<NDIM; i++)
            {
                val *= DimSize[i];
            }
            return val;
        }

        //compute 2^N at compile time
        template <std::size_t N>
        struct PowerOfTwo
        {
            enum { value = 2 * PowerOfTwo<N - 1>::value };
        };

        //compute integer division at compile time
        template <int numerator, int denominator>
        struct Divide
        {
            enum { value = Divide<numerator, 1>::value / Divide<denominator, 1>::value };
        };

        template<std::size_t NDIM> static void
        OffsetsForReversedIndices(const std::size_t* DimSize, std::size_t* ReversedIndex)
        {
            std::size_t total_array_size = HArrayMath::TotalArraySize<NDIM>(DimSize);
            std::size_t index[NDIM];
            for(std::size_t i=0; i<total_array_size; i++)
            {
                HArrayMath::RowMajorIndexFromOffset<NDIM>(i, DimSize, index);
                for(std::size_t j=0; j<NDIM; j++){ index[j] = (DimSize[j] - index[j])%DimSize[j];};
                ReversedIndex[i] = HArrayMath::OffsetFromRowMajorIndex<NDIM>(DimSize, index);
            }
        }

        template<std::size_t NDIM> inline static std::size_t
        MortonZOrderFromRowMajorIndex(const std::size_t* DimSize, const std::size_t* Index)
        {
            std::size_t max_size = PowerOfTwo< Divide<MAX_MORTON_BITS,NDIM>::value >::value;
            //Since the output is limited by MAX_MORTON_BITS
            //indices with values larger than can be stored by
            //MAX_MORTON_BITS/NDIM will be truncated by the bitset constructor,
            //the largest index/dimension size value allowed is
            //2^{MAX_MORTON_BITS/NDIM}, for example with MAX_MORTON_BITS
            //set to 32, and NDIM=4, the max index allowed is 256
            for(std::size_t i=0; i<NDIM; i++)
            {
                if(DimSize[i] >= max_size )
                {
                    std::cout<<"MortonZOrderFromRowMajorIndex: Error, ";
                    std::cout<<"dimension size "<<DimSize[i]<<" exceeds max ";
                    std::cout<<"allowable value of "<<max_size<<".";
                    std::cout<<std::endl;
                    std::exit(1);
                }
            }

            //interleaved bits from all the coordinates
            std::bitset<MAX_MORTON_BITS> interleaved_bits;
            //now compute the bits of each coordinate and insert them into the interleaved bits
            for(std::size_t i=0; i<NDIM; i++)
            {
                std::bitset<  Divide<MAX_MORTON_BITS,NDIM>::value  > coord_bits(Index[i]);
                for(std::size_t j=0; j < Divide<MAX_MORTON_BITS,NDIM>::value ; j++)
                {
                    interleaved_bits[j*NDIM + i] = coord_bits[j];
                }
            }

            //now convert the value of the interleaved bits to int
            //this cast should be safe since MAX_MORTON_BITS is limited to the size of int
            return static_cast<std::size_t>(interleaved_bits.to_ulong());
        }

        template<std::size_t NDIM> inline static std::size_t
        MortonZOrderFromOffset(std::size_t offset, const std::size_t* DimSize)
        {
            std::size_t index[NDIM];
            RowMajorIndexFromOffset<NDIM>(offset, DimSize, index);
            return MortonZOrderFromRowMajorIndex<NDIM>(DimSize, index);
        }


};


//specialization for base case of power of two
template <>
struct HArrayMath::PowerOfTwo<0>
{
    enum { value = 1 };
};

//specialization for base case of divide
template <int numerator>
struct HArrayMath::Divide<numerator, 1>
{
    enum { value = numerator };
};



}//end of namespace

#endif /* HArrayMath_H__ */
