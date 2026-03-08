#ifndef MHO_EndZeroPadderOptimized_HH__
#define MHO_EndZeroPadderOptimized_HH__

#include <bitset>
#include <cstring>
#include <set>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_EndZeroPadderOptimized.hh
 *@class MHO_EndZeroPadderOptimized
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Aug 11 13:35:43 2023 -0400
 *@brief Pads out the end of a multidimensional array with zeros
 */

/**
 * @brief Class MHO_EndZeroPadderOptimized
 */
template< typename XArgType > class MHO_EndZeroPadderOptimized: public MHO_UnaryOperator< XArgType >
{
    public:
        MHO_EndZeroPadderOptimized()
        {
            fPaddingFactor = 1;
            fPaddedSize = 0;
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
            {
                fInputDimensionSize[i] = 0;
                fOutputDimensionSize[i] = 0;
                fAxesToXForm[i] = true;
            }

            fIsValid = false;
            fInitialized = false;
            fFlipped = false;           //chooses which end we pad
            fNormFXMode = true;         //when enabled, then when flipped, copies the last element to N/2
            fPreserveWorkspace = false; //when false tmp workspace will be destroyed after every use
            fCopyTags = true;           //copy tags if available is enabled by default
            fTmpWorkspace = nullptr;
            fHasPaddedAxis = false;
            fInnermostPaddedAxis = 0;
            fBlockElems = 0;
            fNRows = 0;
            fTotalInputElems = 0;
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
            {
                fInStride[i] = 0;
                fOutStride[i] = 0;
            }
        };

        virtual ~MHO_EndZeroPadderOptimized() { delete fTmpWorkspace; };

        /**
         * @brief Setter for padding factor, the factor M by which the new array will be extended
         * (original array, length N, new array length NM)
         *
         * @param factor New length factor for extended array
         * @note This is a virtual function.
         */
        virtual void SetPaddingFactor(std::size_t factor) { fPaddingFactor = factor; };

        /**
         * @brief Setter for padded size, instead of a multiplicative factor,
         * the original array, length N is padded out ot the new specified length M
         *
         * @param new_size New padded size of type std::size_t
         * @note This is a virtual function.
         */
        virtual void SetPaddedSize(std::size_t new_size)
        {
            fPaddedSize = new_size;
            fPaddingFactor = 1;
        }

        /**
         * @brief Setter for end padded, zero padding from end of data out to end of the array
         * @note This is a virtual function.
         */
        virtual void SetEndPadded() { fFlipped = false; }; //

        /**
         * @brief Setter for reverse end padded, place data at end of array and zero pad out to start
         * @note This is a virtual function.
         */
        virtual void SetReverseEndPadded() { fFlipped = true; };

        /**
         * @brief Disables Normal Mapping FX Mode by setting fNormFXMode to false. UNUSED - TODO REMOVE ME!
         * @note This is a virtual function.
         */
        virtual void DisableNormFXMode() { fNormFXMode = false; };

        /**
         * @brief Enables Normalized FX Mode by setting fNormFXMode to true. UNUSED - TODO REMOVE ME!
         * @note This is a virtual function.
         */
        virtual void EnableNormFXMode() { fNormFXMode = true; };

        /**
         * @brief Sets a flag to preserve workspace memory after execution.
         * @note This is a virtual function.
         */
        virtual void PreserveWorkspace()
        {
            fPreserveWorkspace = true;
        } //keep the memory reserved for the workspace around after exectution

        /**
         * @brief Sets preserve workspace flag to false, delete memory after execution
         * @note This is a virtual function.
         */
        virtual void DoNotPreserveWorkspace() { fPreserveWorkspace = false; }

        /**
         * @brief Disables copying tags by setting fCopyTags to false.
         * @note This is a virtual function.
         */
        virtual void DisableTagCopy() { fCopyTags = false; }

        /**
         * @brief Enables copying of tags.
         * @note This is a virtual function.
         */
        virtual void EnableTagCopy() { fCopyTags = true; }

        /**
         * @brief Selects all axes for transformation.
         * sometimes we may want to select/deselect particular dimensions of the x-form
         * default is to transform along every dimension, but that may not always be needed
         */
        void SelectAllAxes()
        {
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
            {
                fAxesToXForm[i] = true;
            }
        }

        /**
         * @brief Deselects all axes by setting each axis to false.
         */
        void DeselectAllAxes()
        {
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
            {
                fAxesToXForm[i] = false;
            }
        }

        /**
         * @brief Selects an axis for transformation if its index is within the array rank.
         *
         * @param axis_index Index of the axis to select.
         */
        void SelectAxis(std::size_t axis_index)
        {
            if(axis_index < XArgType::rank::value)
            {
                fAxesToXForm[axis_index] = true;
            }
            else
            {
                msg_error("operators", "Cannot transform axis with index: " << axis_index << "for array with rank: "
                                                                            << XArgType::rank::value << eom);
            }
        }

    protected:
        /**
         * @brief Initializes in-place by creating a temporary workspace and calling InitializeOutOfPlace.
         *
         * @param in Input argument of type XArgType*
         * @return Result of InitializeOutOfPlace function call
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArgType* in)
        {
            if(fTmpWorkspace == nullptr)
            {
                fTmpWorkspace = new XArgType();
            }
            return InitializeOutOfPlace(in, fTmpWorkspace);
        }

        /**
         * @brief Executes operation in-place by copying temporary workspace back to input object.
         *
         * @param in Input object of type XArgType* that will be modified in-place.
         * @return Boolean status indicating success or failure of the operation.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArgType* in)
        {
            bool status = ExecuteOutOfPlace(in, fTmpWorkspace);
            //"in-place" execution requires a copy from the workspace back to the object we are modifying
            in->Copy(*fTmpWorkspace);
            if(!fPreserveWorkspace)
            {
                //destroy the temporary workspace when we are done
                delete fTmpWorkspace;
                fTmpWorkspace = nullptr;
            }
            return status;
        }

        /**
         * @brief Initializes out-of-place processing for input and output arrays.
         *
         * @param in Pointer to constant input array of type XArgType
         * @param out Pointer to output array of type XArgType
         * @return Boolean indicating successful initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out)
        {
            if(in != nullptr && out != nullptr && in != out)
            {
                fIsValid = true;
            }

            if(fIsValid)
            {
                //output dimensions must be factor of fPaddingFactor bigger than input dims
                in->GetDimensions(fInputDimensionSize);
                out->GetDimensions(fOutputDimensionSize);
                ConditionallyResizeOutput(in->GetDimensionArray(), out);
                PrecomputeOffsetTables();
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }

        /**
         * @brief Function ExecuteOutOfPlace
         *
         * @param in (const XArgType*)
         * @param out (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out)
        {
            if(fIsValid && fInitialized)
            {
                profiler_scope();
                constexpr std::size_t RANK = XArgType::rank::value;
                const auto* in_data = in->GetData();
                auto* out_data = out->GetData();

                if(!fFlipped)
                {
                    //zero padding is placed at the end of the array
                    out->ZeroArray();
                    if(fHasPaddedAxis)
                    {
                        //row-wise memcpy: each "row" is a contiguous block starting at the
                        //innermost padded axis (fInnermostPaddedAxis); outer indices are
                        //tracked incrementally to avoid per-element integer divisions
                        const std::size_t block_bytes = fBlockElems * sizeof(*in_data);
                        std::size_t idx[RANK] = {};
                        std::size_t in_offset = 0, out_offset = 0;
                        for(std::size_t row = 0; row < fNRows; row++)
                        {
                            std::memcpy(out_data + out_offset, in_data + in_offset, block_bytes);
                            //increment outer indices (d < fInnermostPaddedAxis) with carry
                            for(int d = (int)fInnermostPaddedAxis - 1; d >= 0; d--)
                            {
                                idx[d]++;
                                in_offset += fInStride[d];
                                out_offset += fOutStride[d];
                                if(idx[d] < fInputDimensionSize[d]) break;
                                idx[d] = 0;
                                in_offset -= fInputDimensionSize[d] * fInStride[d];
                                out_offset -= fInputDimensionSize[d] * fOutStride[d];
                            }
                        }
                    }
                    else
                    {
                        //no axes are padded: in and out have the same dimensions, just copy
                        std::memcpy(out_data, in_data, fTotalInputElems * sizeof(*in_data));
                    }
                }
                else
                {
                    //zero padding is placed at the start of the array (flipped mode)
                    //use precomputed per-axis offset tables to avoid per-element index arithmetic
                    out->ZeroArray();
                    std::size_t axis_out_offset[RANK];
                    std::size_t out_flat = 0;
                    for(std::size_t d = 0; d < RANK; d++)
                    {
                        axis_out_offset[d] = fOutAxisOffset[d][0];
                        out_flat += axis_out_offset[d];
                    }
                    std::size_t idx[RANK] = {};
                    for(std::size_t n = 0; n < fTotalInputElems; n++)
                    {
                        out_data[out_flat] = in_data[n];
                        //increment innermost-first with carry, updating out_flat incrementally
                        for(int d = (int)RANK - 1; d >= 0; d--)
                        {
                            idx[d]++;
                            if(idx[d] < fInputDimensionSize[d])
                            {
                                std::size_t new_contrib = fOutAxisOffset[d][idx[d]];
                                out_flat = out_flat - axis_out_offset[d] + new_contrib;
                                axis_out_offset[d] = new_contrib;
                                break;
                            }
                            else
                            {
                                idx[d] = 0;
                                std::size_t new_contrib = fOutAxisOffset[d][0];
                                out_flat = out_flat - axis_out_offset[d] + new_contrib;
                                axis_out_offset[d] = new_contrib;
                                //continue carry to next outer dimension
                            }
                        }
                    }
                }

                IfTableTransformAxis(in, out);
                return true;
            }
            else
            {
                msg_error("operators", "Array dimensions are not valid or intialization failed. Aborting zero padding." << eom);
                return false;
            }
        }

    private:
        //default...does nothing
        /**
         * @brief Applies transformations to all axes of an input XArgType table and copies tags.
         *
         * @tparam XCheckType Template parameter XCheckType
         * @param !in Parameter description
         * @param !out Parameter description
         */
        template< typename XCheckType = XArgType >
        typename std::enable_if< !std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableTransformAxis(const XArgType* /*!in*/, XArgType* /*!out*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        template< typename XCheckType = XArgType >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableTransformAxis(const XArgType* in, XArgType* out)
        {
            for(size_t i = 0; i < XArgType::rank::value; i++) //apply to all axes
            {
                TransformAxis axis_transformer(fAxesToXForm[i], fFlipped, fCopyTags);
                apply_at2< typename XArgType::axis_pack_tuple_type, TransformAxis >(*in, *out, i, axis_transformer);
            }
            out->CopyTags(*in); //make sure the table tags get copied
        }

        class TransformAxis
        {
            public:
                TransformAxis(bool modify, bool flipped, bool copy_tags)
                    : fModify(modify), fFlipped(flipped), fCopyTags(copy_tags){};

                ~TransformAxis(){};

                //generic axis, do nothing
                template< typename XAxisType > void operator()(const XAxisType& axis1, XAxisType& axis2)
                {
                    if(!fCopyTags)
                    {
                        CopyLabelsWithoutTags(axis1, axis2);
                    }
                    else
                    {
                        axis2.Copy(axis1);
                    }
                    if(!fModify)
                    {
                        return;
                    } //just copy this axis
                };

                void operator()(const MHO_Axis< double >& axis1, MHO_Axis< double >& axis2)
                {
                    std::size_t ax1_size = axis1.GetSize();
                    std::size_t ax2_size = axis2.GetSize();
                    if(!fCopyTags)
                    {
                        CopyLabelsWithoutTags(axis1, axis2);
                    }
                    else
                    {
                        axis2.Copy(axis1);
                    }
                    if(!fModify)
                    {
                        return;
                    } //just copy this axis
                    axis2.Resize(ax2_size);
                    //assumes uniform labeling, probably ok as we only need this for FFTs
                    double delta = axis1(1) - axis1(0);
                    if(!fFlipped)
                    {
                        for(std::size_t i = 0; i < ax1_size; i++)
                        {
                            axis2(i) = axis1(i);
                        }
                        for(std::size_t i = ax1_size; i < ax2_size; i++)
                        {
                            axis2(i) = axis1(ax1_size - 1) + (i - (ax1_size - 1)) * delta;
                        }
                    }
                    else
                    {
                        for(std::size_t i = 0; i < ax1_size; i++)
                        {
                            axis2(i) = axis1(ax1_size - 1 - i);
                        }
                        for(std::size_t i = ax1_size; i < ax2_size; i++)
                        {
                            axis2(i) = axis1(0) - (i - (ax1_size - 1)) * delta;
                        }
                    }
                }

                void operator()(const MHO_Axis< float >& axis1, MHO_Axis< float >& axis2)
                {
                    std::size_t ax1_size = axis1.GetSize();
                    std::size_t ax2_size = axis2.GetSize();
                    if(!fCopyTags)
                    {
                        CopyLabelsWithoutTags(axis1, axis2);
                    }
                    else
                    {
                        axis2.Copy(axis1);
                    }
                    if(!fModify)
                    {
                        return;
                    } //just copy this axis
                    axis2.Resize(ax2_size);
                    //assumes uniform labeling, probably ok as we only need this for FFTs
                    double delta = axis1(1) - axis1(0);
                    if(!fFlipped)
                    {
                        for(std::size_t i = 0; i < ax1_size; i++)
                        {
                            axis2(i) = axis1(i);
                        }
                        for(std::size_t i = ax1_size; i < ax2_size; i++)
                        {
                            axis2(i) = axis1(ax1_size - 1) + (i - (ax1_size - 1)) * delta;
                        }
                    }
                    else
                    {
                        for(std::size_t i = 0; i < ax1_size; i++)
                        {
                            axis2(i) = axis1(ax1_size - 1 - i);
                        }
                        for(std::size_t i = ax1_size; i < ax2_size; i++)
                        {
                            axis2(i) = axis1(0) - (i - (ax1_size - 1)) * delta;
                        }
                    }
                }

            private:
                template< typename XAxisType > void CopyLabelsWithoutTags(const XAxisType& axis1, XAxisType& axis2)
                {
                    //just copy axis labels
                    std::size_t ax1_size = axis1.GetSize();
                    std::size_t ax2_size = axis2.GetSize();
                    std::size_t s = std::min(ax1_size, ax2_size);
                    for(std::size_t i = 0; i < s; i++)
                    {
                        axis2(i) = axis1(i);
                    }
                }

                bool fModify;
                bool fFlipped;
                bool fCopyTags;
        };

        void PrecomputeOffsetTables()
        {
            constexpr std::size_t RANK = XArgType::rank::value;

            //compute element strides for both input and output arrays
            fInStride[RANK - 1] = 1;
            fOutStride[RANK - 1] = 1;
            for(int d = (int)RANK - 2; d >= 0; d--)
            {
                fInStride[d] = fInStride[d + 1] * fInputDimensionSize[d + 1];
                fOutStride[d] = fOutStride[d + 1] * fOutputDimensionSize[d + 1];
            }

            //find the innermost (highest-index) axis that is being padded
            fHasPaddedAxis = false;
            fInnermostPaddedAxis = 0;
            for(std::size_t d = 0; d < RANK; d++)
            {
                if(fAxesToXForm[d])
                {
                    fInnermostPaddedAxis = d;
                    fHasPaddedAxis = true;
                }
            }

            //for the !fFlipped path: block size (contiguous elements per row) and row count
            fBlockElems = 1;
            fNRows = 1;
            if(fHasPaddedAxis)
            {
                for(std::size_t d = fInnermostPaddedAxis; d < RANK; d++)
                    fBlockElems *= fInputDimensionSize[d];
                for(std::size_t d = 0; d < fInnermostPaddedAxis; d++)
                    fNRows *= fInputDimensionSize[d];
            }

            //total input elements (used for fFlipped path and no-padded-axis fast copy)
            fTotalInputElems = 1;
            for(std::size_t d = 0; d < RANK; d++)
                fTotalInputElems *= fInputDimensionSize[d];

            //for the fFlipped path: precompute per-axis, per-index contributions to the
            //output flat offset, so Execute needs only a table lookup + addition per element
            fOutAxisOffset.resize(RANK);
            for(std::size_t d = 0; d < RANK; d++)
            {
                fOutAxisOffset[d].resize(fInputDimensionSize[d]);
                for(std::size_t k = 0; k < fInputDimensionSize[d]; k++)
                {
                    if(!fAxesToXForm[d])
                    {
                        fOutAxisOffset[d][k] = k * fOutStride[d];
                    }
                    else if(!fNormFXMode)
                    {
                        fOutAxisOffset[d][k] = (fOutputDimensionSize[d] - 1 - k) * fOutStride[d];
                    }
                    else
                    {
                        //fNormFXMode: n=0 maps to N/2 for compatibility with norm_fx
                        if(k == 0)
                            fOutAxisOffset[d][k] = (fOutputDimensionSize[d] / 2) * fOutStride[d];
                        else
                            fOutAxisOffset[d][k] = (fOutputDimensionSize[d] - k) * fOutStride[d];
                    }
                }
            }
        }

        void ConditionallyResizeOutput(const std::array< std::size_t, XArgType::rank::value >& dims, XArgType* out)
        {
            auto out_dim = out->GetDimensionArray();
            bool have_to_resize = false;
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
            {
                if(fAxesToXForm[i])
                {
                    if(dims[i] * fPaddingFactor != out_dim[i])
                    {
                        have_to_resize = true;
                        out_dim[i] = dims[i] * fPaddingFactor;
                    }
                    if(fPaddingFactor == 1)
                    {
                        if(dims[i] != fPaddedSize)
                        {
                            have_to_resize = true;
                            out_dim[i] = fPaddedSize;
                        }
                    }
                }
                else
                {
                    if(dims[i] != out_dim[i])
                    {
                        have_to_resize = true;
                        out_dim[i] = dims[i];
                    }
                }
            }
            if(have_to_resize)
            {
                out->Resize(&(out_dim[0]));
            }
            out->GetDimensions(fOutputDimensionSize);
        }

        bool fIsValid;
        bool fInitialized;
        bool fFlipped;
        bool fNormFXMode;
        bool fPreserveWorkspace;
        bool fCopyTags;

        std::size_t fPaddingFactor;
        std::size_t fPaddedSize;
        std::size_t fInputDimensionSize[XArgType::rank::value];
        std::size_t fOutputDimensionSize[XArgType::rank::value];
        bool fAxesToXForm[XArgType::rank::value];

        //precomputed for fast ExecuteOutOfPlace
        bool fHasPaddedAxis;
        std::size_t fInnermostPaddedAxis;  //max axis index where fAxesToXForm[d]==true
        std::size_t fBlockElems;           //contiguous input elements per row (!fFlipped path)
        std::size_t fNRows;                //number of rows to copy (!fFlipped path)
        std::size_t fTotalInputElems;      //total number of input elements
        std::size_t fInStride[XArgType::rank::value];
        std::size_t fOutStride[XArgType::rank::value];
        std::vector< std::vector< std::size_t > > fOutAxisOffset; //[dim][idx] -> out flat offset contribution (fFlipped path)

        XArgType* fTmpWorkspace;
};

} // namespace hops

#endif /*! MHO_EndZeroPadderOptimized_H__ */
