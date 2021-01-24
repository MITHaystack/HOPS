#ifndef MHONDArrayMath_HH__
#define MHONDArrayMath_HH__

#include <cmath>
#include <cstddef>

namespace hops{

class MHONDArrayMath
{
    public:
        MHONDArrayMath(){};
        virtual ~MHONDArrayMath(){};

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
        template<std::size_t RANK> inline static std::size_t
        OffsetFromRowMajorIndex(const std::size_t* DimSize, const std::size_t* Index)
        {
            std::size_t val = Index[0];
            for(std::size_t i=1; i<RANK; i++)
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
        template<std::size_t RANK> inline static std::size_t
        StrideFromRowMajorIndex(std::size_t selected_dim, const std::size_t* DimSize)
        {
            std::size_t val = 1;
            for(std::size_t i=0; i<RANK; i++)
            {
                if(i > selected_dim){val *= DimSize[i];};
            }
            return val;
        }

        //for a multidimensional array (using row major indexing) which has the
        //dimensions specified in DimSize, this function computes the indices of
        //the elements which has the given offset from the first element
        template<std::size_t RANK> inline static void
        RowMajorIndexFromOffset(std::size_t offset, const std::size_t* DimSize, std::size_t* Index)
        {
            std::size_t div[RANK];

            //in row major format the last index varies the fastest
            std::size_t i;
            for(std::size_t d=0; d < RANK; d++)
            {
                i = RANK - d -1;

                if(d == 0)
                {
                    Index[i] = MHONDArrayMath::Modulus(offset, DimSize[i]);
                    div[i] = (offset - Index[i])/DimSize[i];
                }
                else
                {
                    Index[i] = MHONDArrayMath::Modulus(div[i+1], DimSize[i]);
                    div[i] = (div[i+1] - Index[i])/DimSize[i];
                }
            }
        }

        //checks if all the indices in Index are in the valid range
        template<std::size_t RANK> inline static bool
        CheckIndexValidity(const std::size_t* DimSize, const std::size_t* Index)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                if(Index[i] >= DimSize[i]){return false;};
            }
            return true;
        };


        //given the dimensions of an array, computes its total size, assuming all dimensions are non-zero
        template<std::size_t RANK> inline static std::size_t
        TotalArraySize(const std::size_t* DimSize)
        {
            std::size_t val = 1;
            for(std::size_t i=0; i<RANK; i++)
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

        template<std::size_t RANK> static void
        OffsetsForReversedIndices(const std::size_t* DimSize, std::size_t* ReversedIndex)
        {
            std::size_t total_array_size = MHONDArrayMath::TotalArraySize<RANK>(DimSize);
            std::size_t index[RANK];
            for(std::size_t i=0; i<total_array_size; i++)
            {
                MHONDArrayMath::RowMajorIndexFromOffset<RANK>(i, DimSize, index);
                for(std::size_t j=0; j<RANK; j++){ index[j] = (DimSize[j] - index[j])%DimSize[j];};
                ReversedIndex[i] = MHONDArrayMath::OffsetFromRowMajorIndex<RANK>(DimSize, index);
            }
        }

        //increment the multi-dimensional indices by 1
        template<std::size_t RANK> static bool
        IncrementIndices(const std::size_t* DimSize, std::size_t* Index)
        {
            for(std::size_t i=1; i <= RANK; i++)
            {
                if(++Index[RANK-i] < DimSize[RANK-i]){return true;}
                else{Index[RANK-i] = 0;}
            }
            //if we have reached here we have overflowed the 0-th dimension
            return false;
        }

        //increment the multi-dimensional indices by the amount in diff
        template<std::size_t RANK> static bool
        IncrementIndices(const std::size_t* DimSize, std::size_t* Index, std::size_t diff)
        {
            std::size_t offset = OffsetFromRowMajorIndex<RANK>(DimSize,Index);
            offset += diff;
            if(offset < TotalArraySize<RANK>(DimSize) )
            {
                RowMajorIndexFromOffset<RANK>(offset, DimSize, Index);
                return true;
            }
            //cannot increment past the end
            return false;
        }


        template<std::size_t RANK> static bool
        DecrementIndices(const std::size_t* DimSize, std::size_t* Index)
        {
            for(std::size_t i=1; i <= RANK; i++)
            {
                if(Index[RANK-i] > 0){--Index[RANK-i];return true;}
                else{Index[RANK-i] = DimSize[RANK-i]-1;}
            }
            //if we reach here we have underflowed the 0-th dimension
            return false;
        }

        template<std::size_t RANK> static bool
        DecrementIndices(const std::size_t* DimSize, std::size_t* Index, std::size_t diff)
        {
            std::size_t offset = OffsetFromRowMajorIndex<RANK>(DimSize,Index);
            if(diff < offset )
            {
                offset -= diff;
                RowMajorIndexFromOffset<RANK>(offset, DimSize, Index);
                return true;
            }
            //cannot decrement more than the offset from start
            return false;
        }

};


//specialization for base case of power of two
template <>
struct MHONDArrayMath::PowerOfTwo<0>
{
    enum { value = 1 };
};

//specialization for base case of divide
template <int numerator>
struct MHONDArrayMath::Divide<numerator, 1>
{
    enum { value = numerator };
};


}//end of namespace

#endif /* MHONDArrayMath_HH__ */
