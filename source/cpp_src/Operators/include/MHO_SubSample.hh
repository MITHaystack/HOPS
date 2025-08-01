#ifndef MHO_SubSample_HH__
#define MHO_SubSample_HH__

#include <algorithm>
#include <cstdint>

#include "MHO_Message.hh"
#include "MHO_Meta.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_SubSample.hh
 *@class MHO_SubSample
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Aug 24 15:18:04 2021 -0400
 *@brief
 * Sub-samples an array at the specified stride (e.g. select every-other point)
 * Can only be applied to a single-axis at a time.
 */

/**
 * @brief Class MHO_SubSample
 */
template< class XArrayType > class MHO_SubSample: public MHO_UnaryOperator< XArrayType >
{
    public:
        MHO_SubSample()
        {
            fInitialized = false;
            fDimIndex = 0;
            fStride = 1;
        };

        virtual ~MHO_SubSample(){};

        /**
         * @brief set the axis to sub sample, and the stride at which samples are selected
         *
         * @param dimension_index Index of the dimension to set (must be less than array rank).
         * @param stride Stride at which samples are selected along the specified dimension.
         */
        void SetDimensionAndStride(std::size_t dimension_index, std::size_t stride)
        {
            if(dimension_index < XArrayType::rank::value)
            {
                fDimIndex = dimension_index;
                fStride = stride;
            }
            else
            {
                msg_error("operators", "error, cannot select dimension: " << dimension_index << ", exceeds array rank." << eom);
            }
            fInitialized = false;
        }

    protected:
        /**
         * @brief Initializes in-place execution by calling InitializeOutOfPlace with workspace and returning its result.
         *
         * @param in Input array of type XArrayType*
         * @return Boolean indicating success/failure of initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArrayType* in) override { return InitializeOutOfPlace(in, &fWorkspace); }

        /**
         * @brief Executes operation in-place and updates input array from workspace.
         *
         * @param in Input array to be modified in-place.
         * @return Status of the execution operation.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            bool status = ExecuteOutOfPlace(in, &fWorkspace);
            //"in-place" execution requires a copy from the workspace back to the object we are modifying
            in->Copy(fWorkspace);

            // auto workspace_dim = fWorkspace.GetDimensionArray();
            // for(std::size_t i=0; i<workspace_dim.size(); i++){workspace_dim[i] = 0;}
            // fWorkspace.Resize(&(workspace_dim[0]));

            return status;
        }

        /**
         * @brief Initializes output array out-of-place using input array in.
         *
         * @param in Input array of type XArrayType
         * @param out Output array of type XArrayType
         * @return True if initialization is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                //check that the dimension we are sub-sampling is divisable by the stride
                if(in->GetDimension(fDimIndex) % fStride != 0)
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

        /**
         * @brief Function ExecuteOutOfPlace
         *
         * @param in (const XArrayType*)
         * @param out (XArrayType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            size_t index[XArrayType::rank::value];
            size_t non_active_dimension_size[XArrayType::rank::value - 1];
            size_t non_active_dimension_value[XArrayType::rank::value - 1];
            size_t non_active_dimension_index[XArrayType::rank::value - 1];

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
            for(size_t n = 0; n < n_srow; n++)
            {
                //invert place in list to obtain indices of block in array
                MHO_NDArrayMath::RowMajorIndexFromOffset< XArrayType::rank::value - 1 >(n, non_active_dimension_size,
                                                                                        non_active_dimension_value);

                //copy the value of the non-active dimensions in to index
                for(size_t i = 0; i < XArrayType::rank::value - 1; i++)
                {
                    index[non_active_dimension_index[i]] = non_active_dimension_value[i];
                }
                index[d] = 0; //always at start of row, for dim we are sampling

                //locate the start of this row
                size_t in_data_location;
                in_data_location =
                    MHO_NDArrayMath::OffsetFromRowMajorIndex< XArrayType::rank::value >(in->GetDimensions(), index);

                //locate the start of this row
                size_t out_data_location;
                out_data_location =
                    MHO_NDArrayMath::OffsetFromRowMajorIndex< XArrayType::rank::value >(out->GetDimensions(), index);

                //now sample the array by the specified amount
                auto in_iter = in->cstride_iterator_at(in_data_location, fStride * in_stride);
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

            IfTableSubSampleAxis(in, out);

            return true;
        }

    private:
        /**
         * @brief Calculates output dimensions by dividing the specified dimension index by stride and returning the modified dimension array.
         *
         * @param in Input XArrayType object from which to calculate output dimensions.
         * @return Modified dimension array with the specified dimension divided by stride.
         */
        std::array< std::size_t, XArrayType::rank::value > DetermineOutputDimensions(const XArrayType* in)
        {
            auto in_dim = in->GetDimensionArray();
            in_dim[fDimIndex] /= fStride;
            return in_dim;
        }

        /**
         * @brief Checks and conditionally resizes output array dimensions if they differ from input.
         *
         * @param dims Input dimension array to compare with current output dimensions.
         * @param out (XArrayType*)
         */
        void ConditionallyResizeOutput(const std::array< std::size_t, XArrayType::rank::value >& dims, XArrayType* out)
        {
            auto out_dim = out->GetDimensionArray();
            bool have_to_resize = false;
            for(std::size_t i = 0; i < XArrayType::rank::value; i++)
            {
                if(out_dim[i] != dims[i])
                {
                    have_to_resize = true;
                }
            }
            if(have_to_resize)
            {
                out->Resize(&(dims[0]));
            }
        }

        //default...does nothing
        /**
         * @brief Sub-samples a specified axis of an input XArrayType and stores the result in output.
         *
         * @tparam XCheckType Template parameter XCheckType
         * @param !in Input XArrayType to be sub-sampled
         * @param !out Output XArrayType after sub-sampling
         */
        template< typename XCheckType = XArrayType >
        typename std::enable_if< !std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableSubSampleAxis(const XArrayType* /*!in*/, XArrayType* /*!out*/){};

        /**
         * @brief Sub-samples a specified axis of an input XArrayType and stores the result in output
         * uses SFINAE to generate specialization for MHO_TableContainer types
         *
         * @tparam XCheckType Template parameter XCheckType
         * @param in Input XArrayType to be sub-sampled
         * @param out Output XArrayType where results are stored
         */
        template< typename XCheckType = XArrayType >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableSubSampleAxis(const XArrayType* in, XArrayType* out)
        {
            for(size_t i = 0; i < XArrayType::rank::value; i++) //apply to all axes
            {
                std::size_t stride = 1; //non-sub-sampled axis are just copied directly (stride of 1)
                if(i == fDimIndex)
                {
                    stride = fStride;
                }
                SubSampleAxis axis_sub_sampler(stride);
                apply_at2< typename XArrayType::axis_pack_tuple_type, SubSampleAxis >(*in, *out, i, axis_sub_sampler);
            }
            out->CopyTags(*in); //make sure the table tags get copied
        }

        /**
         * @brief Class SubSampleAxis
         */
        class SubSampleAxis
        {
            public:
                SubSampleAxis(std::size_t stride): fStride(stride){};
                ~SubSampleAxis(){};

                template< typename XAxisType > void operator()(const XAxisType& axis1, XAxisType& axis2)
                {
                    if(fStride == 1)
                    {
                        axis2.Copy(axis1);
                    }
                    else
                    {
                        TODO_FIXME_MSG("ensure that only the proper index tags are selected/copied here.")
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
                }

            private:
                std::size_t fStride;
        };

        bool fInitialized;
        std::size_t fDimIndex;
        std::size_t fStride;
        XArrayType fWorkspace;
};

}; // namespace hops

#endif /*! MHO_SubSample_H__ */
