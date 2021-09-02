#ifndef MHO_CyclicRotator_HH__
#define MHO_CyclicRotator_HH__

#include <algorithm>
#include <cstdint>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_NDArrayOperator.hh"



/*
*File: MHO_CyclicRotator.hh
*Class: MHO_CyclicRotator
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: 
* Applies a cyclic rotation on the contents on a multidimensional array 
* by some specified offset for each dimension.
*/


namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHO_CyclicRotator: public MHO_NDArrayOperator<XInputArrayType, XOutputArrayType >
{
    public:

        static_assert(XOutputArrayType::rank::value == XInputArrayType::rank::value, "Input/Output array ranks are not equal.");

        MHO_CyclicRotator():
            fInitialized(false)
        {
            for(std::size_t i=0; i<XInputArrayType::rank::value; i++)
            {
                fDimensionSize[i] = 0;
                fOffsets[i]=0;
            }
        };

        virtual ~MHO_CyclicRotator(){};

        //set the offset for the cyclic rotation in each dimension (default is zero...do nothing)
        void SetOffset(std::size_t dimension_index, int64_t offset_value)
        {
            //A negative offset_value results in a "right" rotation: rot by  1: [0 1 2 3] -> [3 0 1 2]
            //A positive offset_value results in a "left" rotation: rot by -1: [0 1 2 3] -> [1 2 3 0]
            if(dimension_index < XInputArrayType::rank::value)
            {
                fOffsets[dimension_index] = offset_value;
            }
            else 
            {
                msg_error("operators", "error, offset for dimension: "<<dimension_index<<", exceeds array rank." << eom);
            }
            fInitialized = false;
        }

        virtual bool Initialize() override
        {
            fInitialized = false;
            if(this->fInput != nullptr && this->fOutput != nullptr)
            {
                this->fInput->GetDimensions(fDimensionSize);
                //only need to change output size if in != out and size is different
                if(this->fInput != this->fOutput)
                {
                    std::size_t in_dim[XInputArrayType::rank::value];
                    std::size_t out_dim[XOutputArrayType::rank::value];
                    this->fInput->GetDimensions(in_dim);
                    this->fOutput->GetDimensions(out_dim);

                    bool have_to_resize = false;
                    for(std::size_t i=0; i<XInputArrayType::rank::value; i++)
                    {
                        if(out_dim[i] != in_dim[i]){have_to_resize = true; break;}
                    }

                    if(have_to_resize){this->fOutput->Resize(in_dim);}
                }
                fInitialized = true;
            }
            return fInitialized;
        }

        virtual bool ExecuteOperation() override
        {
            if(fInitialized)
            {
                for(std::size_t i=0; i<XInputArrayType::rank::value; i++)
                {
                    fModuloOffsets[i] = positive_modulo(fOffsets[i], fDimensionSize[i]);
                }

                if(this->fInput == this->fOutput) 
                {
                    size_t index[XInputArrayType::rank::value];
                    size_t non_active_dimension_size[XInputArrayType::rank::value-1];
                    size_t non_active_dimension_value[XInputArrayType::rank::value-1];
                    size_t non_active_dimension_index[XInputArrayType::rank::value-1];

                    //select the dimension on which to perform the FFT
                    for(size_t d = 0; d < XInputArrayType::rank::value; d++)
                    {
                        if(fOffsets[d] !=0)
                        {
                            //now we loop over all dimensions not specified by d
                            //first compute the number of arrays we need to rotate
                            size_t n_rot = 1;
                            size_t count = 0;
                            size_t stride = this->fInput->GetStride(d);
                            for(size_t i = 0; i < XInputArrayType::rank::value; i++)
                            {
                                if(i != d)
                                {
                                    n_rot *= fDimensionSize[i];
                                    non_active_dimension_index[count] = i;
                                    non_active_dimension_size[count] = fDimensionSize[i];
                                    count++;
                                }
                            }

                            //loop over the number of rotations to perform
                            for(size_t n=0; n<n_rot; n++)
                            {
                                //invert place in list to obtain indices of block in array
                                MHO_NDArrayMath::RowMajorIndexFromOffset<XInputArrayType::rank::value-1>(n, non_active_dimension_size, non_active_dimension_value);

                                //copy the value of the non-active dimensions in to index
                                for(size_t i=0; i<XInputArrayType::rank::value-1; i++)
                                {
                                    index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
                                }

                                //locate the start of this row
                                size_t data_location;
                                index[d] = 0;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XInputArrayType::rank::value>(fDimensionSize, index);

                                //now rotate the array by the specified amount
                                auto it_first = this->fInput->stride_iterator_at(data_location, stride);
                                auto it_nfirst = it_first + fModuloOffsets[d];
                                auto it_end = it_first + fDimensionSize[d];
                                std::rotate(it_first, it_nfirst, it_end);
                            }
                        }
                    }
                    return true;
                }
                else 
                {
                    //first get the indices of the input iterator
                    auto in_iter =  this->fInput->begin();
                    auto in_iter_end = this->fInput->end();
                    const std::size_t* out_dim = this->fOutput->GetDimensions();
                    while( in_iter != in_iter_end)
                    {
                        const std::size_t* in_loc = in_iter.GetIndices();
                        for(std::size_t i=0; i<XInputArrayType::rank::value;i++)
                        {
                            fWorkspace[i] = positive_modulo( in_loc[i] - fModuloOffsets[i], out_dim[i]);
                        }
                        std::size_t out_loc = this->fOutput->GetOffsetForIndices(fWorkspace);
                        (*(this->fOutput))[out_loc] = *in_iter;
                        in_iter++;
                    }
                    return true;
                }
            }
            return false;
        }


    private:

        bool fInitialized;

        //note: using the modulo is a painfully slow to do this
        //TODO FIXME...we ought to check for uint64_t -> int64_t overflows!
        inline int64_t positive_modulo(int64_t i, int64_t n)
        {
            return (i % n + n) % n;
        }

        //offsets to for cyclic rotation
        std::size_t  fDimensionSize[XInputArrayType::rank::value];
        int64_t fOffsets[XInputArrayType::rank::value];
        int64_t fModuloOffsets[XInputArrayType::rank::value];
        std::size_t fWorkspace[XInputArrayType::rank::value];
};

};




#endif /* MHO_CyclicRotator_H__ */
