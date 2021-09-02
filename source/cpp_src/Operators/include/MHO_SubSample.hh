#ifndef MHO_SubSample_HH__
#define MHO_SubSample_HH__

#include <algorithm>
#include <cstdint>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_NDArrayOperator.hh"



/*
*File: MHO_SubSample.hh
*Class: MHO_SubSample
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: 
* Sub-samples an array at the specified stride (e.g. select every-other point)
* Can only be applied to a single-axis at a time.
*/


namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHO_SubSample: public MHO_NDArrayOperator<XInputArrayType, XOutputArrayType >
{
    public:

        static_assert(XOutputArrayType::rank::value == XInputArrayType::rank::value, "Input/Output array ranks are not equal.");

        MHO_SubSample():
            fInitialized(false)
        {
            fDimIndex = 0;
            fStride = 1;
        };

        virtual ~MHO_SubSample(){};

        //set the offset for the cyclic rotation in each dimension (default is zero...do nothing)
        void SetDimensionStride(std::size_t dimension_index, std::size_t stride)
        {
            //A negative offset_value results in a "right" rotation: rot by  1: [0 1 2 3] -> [3 0 1 2]
            //A positive offset_value results in a "left" rotation: rot by -1: [0 1 2 3] -> [1 2 3 0]
            if(dimension_index < XInputArrayType::rank::value)
            {
                fDimIndex = dimension_index;
                fStride = stride;
            }
            else 
            {
                msg_error("operators", "error, cannot select dimension: "<<dimension_index<<", exceeds array rank." << eom);
            }
            fInitialized = false;
        }

        virtual bool Initialize() override
        {
            fInitialized = false;
            if(this->fInput != nullptr && this->fOutput != nullptr)
            {

                //check that the dimension we are sub-sampling is divisable by the stride 
                if(this->fInput->GetDimension(fDimIndex)%fStride != 0)
                {
                    fInitialized = false;
                    msg_error("operators", "dimension to sub-sample must be divisable by stride." << eom);
                    return fInitialized;
                }

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
                        if(i == fDimIndex)
                        {
                            if(out_dim[i] != in_dim[i]/fStride){have_to_resize = true; break;}
                        }
                        else 
                        {
                            if(out_dim[i] != in_dim[i]){have_to_resize = true; break;}
                        }
                    }
                    in_dim[fDimIndex] /= fStride;
                    if(have_to_resize){this->fOutput->Resize(in_dim);}
                    fInitialized = true;
                }
                else 
                {
                    msg_error("operators", "cannot execute sub-sample operation in place." << eom );
                }
            }
            return fInitialized;
        }

        virtual bool ExecuteOperation() override
        {
            if(fInitialized)
            {
                size_t index[XInputArrayType::rank::value];
                size_t non_active_dimension_size[XInputArrayType::rank::value-1];
                size_t non_active_dimension_value[XInputArrayType::rank::value-1];
                size_t non_active_dimension_index[XInputArrayType::rank::value-1];
        
                std::size_t d = fDimIndex;
                //now we loop over all dimensions not specified by d
                //first compute the number of arrays we need to rotate
                size_t n_rot = 1;
                size_t count = 0;
                size_t in_stride = this->fInput->GetStride(d);
                for(size_t i = 0; i < XInputArrayType::rank::value; i++)
                {
                    if(i != d)
                    {
                        n_rot *= this->fInput->GetDimension(i);
                        non_active_dimension_index[count] = i;
                        non_active_dimension_size[count] = this->fInput->GetDimension(i);
                        count++;
                    }
                }

                //loop over the number of rows we need to sub-sample
                for(size_t n=0; n<n_rot; n++)
                {
                    //invert place in list to obtain indices of block in array
                    MHO_NDArrayMath::RowMajorIndexFromOffset<XInputArrayType::rank::value-1>(n, non_active_dimension_size, non_active_dimension_value);

                    //copy the value of the non-active dimensions in to index
                    for(size_t i=0; i<XInputArrayType::rank::value-1; i++)
                    {
                        index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
                    }
                    index[d] = 0; //always at start of row, for dim we are sampling

                    //locate the start of this row
                    size_t in_data_location;
                    in_data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XInputArrayType::rank::value>(this->fInput->GetDimensions(), index);

                    //locate the start of this row
                    size_t out_data_location;
                    out_data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XInputArrayType::rank::value>(this->fOutput->GetDimensions(), index);

                    //now rotate the array by the specified amount
                    auto in_iter = this->fInput->stride_iterator_at(in_data_location, fStride*in_stride);
                    auto out_iter = this->fOutput->stride_iterator_at(out_data_location, this->fOutput->GetStride(d));
                    auto in_end = in_iter + this->fInput->GetDimension(d);
                    auto out_end = out_iter + this->fOutput->GetDimension(d);

                    do
                    {
                        *out_iter = *in_iter;
                        ++in_iter;
                        ++out_iter;
                    }
                    while(in_iter != in_end && out_iter != out_end);
                }
                return true;
            }
            return false;
        }


    private:

        bool fInitialized;
        std::size_t fDimIndex;
        std::size_t fStride;
};

};




#endif /* MHO_SubSample_H__ */
