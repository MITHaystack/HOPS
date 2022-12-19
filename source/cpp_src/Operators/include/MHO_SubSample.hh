#ifndef MHO_SubSample_HH__
#define MHO_SubSample_HH__

#include <algorithm>
#include <cstdint>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"

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

template< class XArrayType>
class MHO_SubSample:
    public MHO_UnaryOperator< XArrayType >
{
    public:

        MHO_SubSample()
        {
            fInitialized = false;
            fDimIndex = 0;
            fStride = 1;
        };

        virtual ~MHO_SubSample(){};

        //set the axis to sub sample, and the stride at which samples are selected
        void SetDimensionAndStride(std::size_t dimension_index, std::size_t stride)
        {
            if(dimension_index < XArrayType::rank::value)
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

    protected:

        virtual bool InitializeInPlace(XArrayType* in) override
        {
            return InitializeOutOfPlace(in, &fWorkspace);
        }

        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            bool status = ExecuteOutOfPlace(in, &fWorkspace);
            //"in-place" execution requires a copy from the workspace back to the object we are modifying
            in->Copy(fWorkspace);
            return status;
        }

        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                //check that the dimension we are sub-sampling is divisable by the stride
                if(in->GetDimension(fDimIndex)%fStride != 0)
                {
                    msg_error("operators", "dimension to sub-sample must be divisable by stride." << eom);
                    return false;
                }
                auto dims = DetermineOutputDimensions(in);
                ConditionallyResizeOutput(dims, out);
                fInitialized = true;
            }
            return true;
        }

        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            size_t index[XArrayType::rank::value];
            size_t non_active_dimension_size[XArrayType::rank::value-1];
            size_t non_active_dimension_value[XArrayType::rank::value-1];
            size_t non_active_dimension_index[XArrayType::rank::value-1];

            std::size_t d = fDimIndex;
            //now we loop over all dimensions not specified by d
            //first compute the number of arrays we need to sub-sample
            size_t n_srow = 1;
            size_t count = 0;
            size_t in_stride = in->GetStride(d);
            for(size_t i = 0; i < XArrayType::rank::value; i++)
            {
                if(i != d)
                {
                    n_srow *= in->GetDimension(i);
                    non_active_dimension_index[count] = i;
                    non_active_dimension_size[count] = in->GetDimension(i);
                    count++;
                }
            }

            //loop over the number of rows we need to sub-sample
            for(size_t n=0; n<n_srow; n++)
            {
                //invert place in list to obtain indices of block in array
                MHO_NDArrayMath::RowMajorIndexFromOffset<XArrayType::rank::value-1>(n, non_active_dimension_size, non_active_dimension_value);

                //copy the value of the non-active dimensions in to index
                for(size_t i=0; i<XArrayType::rank::value-1; i++)
                {
                    index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
                }
                index[d] = 0; //always at start of row, for dim we are sampling

                //locate the start of this row
                size_t in_data_location;
                in_data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArrayType::rank::value>(in->GetDimensions(), index);

                //locate the start of this row
                size_t out_data_location;
                out_data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArrayType::rank::value>(out->GetDimensions(), index);

                //now sample the array by the specified amount
                auto in_iter = in->cstride_iterator_at(in_data_location, fStride*in_stride);
                auto out_iter = out->stride_iterator_at(out_data_location, out->GetStride(d));
                auto in_end = in_iter + in->GetDimension(d);
                auto out_end = out_iter + out->GetDimension(d);

                do
                {
                    *out_iter = *in_iter;
                    ++in_iter;
                    ++out_iter;
                }
                while(in_iter != in_end && out_iter != out_end);
            }

            IfTableSubSampleAxis(in,out);

            return true;
        }




    private:

        std::array<std::size_t, XArrayType::rank::value> DetermineOutputDimensions(const XArrayType* in)
        {
            auto in_dim = in->GetDimensionArray();
            in_dim[fDimIndex] /= fStride;
            return in_dim;
        }

        void ConditionallyResizeOutput(const std::array<std::size_t, XArrayType::rank::value>& dims, XArrayType* out)
        {
            auto out_dim = out->GetDimensionArray();
            bool have_to_resize = false;
            for(std::size_t i=0; i<XArrayType::rank::value; i++)
            {
                if(out_dim[i] != dims[i] ){have_to_resize = true;}
            }
            if(have_to_resize){ out->Resize( &(dims[0]) );}
        }

        //default...does nothing
        template< typename XCheckType = XArrayType >
        typename std::enable_if< !std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableSubSampleAxis(const XArrayType* /*in*/, XArrayType* /*out*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        template< typename XCheckType = XArrayType >
        typename std::enable_if< std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableSubSampleAxis(const XArrayType* in, XArrayType* out)
        {
            SubSampleAxis axis_sub_sampler(fStride);
            apply_at2< typename XArrayType::axis_pack_tuple_type, SubSampleAxis >( *in, *out, fDimIndex, axis_sub_sampler);
            out->CopyTags(*in); //make sure the table tags get copied
        }


        class SubSampleAxis
        {
            public:
                SubSampleAxis(std::size_t stride):
                    fStride(stride)
                {};
                ~SubSampleAxis(){};

                template< typename XAxisType >
                void operator()(const XAxisType& axis1, XAxisType& axis2)
                {
                    axis2.CopyTags(axis1); //copy the axis tags
                    //at this point axis2 should already be re-sized appropriately
                    auto it1 = axis1.cstride_begin(fStride);
                    auto end1 = axis1.cstride_end(fStride);
                    auto it2 = axis2.begin();
                    auto end2 = axis2.end();
                    while(it1 != end1 && it2 != end2)
                    {
                        *it2++ = *it1++;
                    }
                }

            private:
                std::size_t fStride;
        };


        bool fInitialized;
        std::size_t fDimIndex;
        std::size_t fStride;
        XArrayType fWorkspace;
};







};




#endif /* MHO_SubSample_H__ */
