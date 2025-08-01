#ifndef MHO_EndZeroPadder_HH__
#define MHO_EndZeroPadder_HH__

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
 *@file MHO_EndZeroPadder.hh
 *@class MHO_EndZeroPadder
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Aug 11 13:35:43 2023 -0400
 *@brief Pads out the end of a multidimensional array with zeros
 */

/**
 * @brief Class MHO_EndZeroPadder
 */
template< typename XArgType > class MHO_EndZeroPadder: public MHO_UnaryOperator< XArgType >
{
    public:
        MHO_EndZeroPadder()
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
        };

        virtual ~MHO_EndZeroPadder() { delete fTmpWorkspace; };

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
                //zero out the output array
                out->ZeroArray();
                if(!fFlipped)
                {
                    //zero padding is placed at the end of the array
                    auto in_iter = in->cbegin();
                    auto in_iter_end = in->cend();
                    std::array< std::size_t, XArgType::rank::value > in_indices;
                    while(in_iter != in_iter_end)
                    {
                        //copy the input data to the same 'index' location in the output array

                        //get the input indices for each dimension
                        MHO_NDArrayMath::RowMajorIndexFromOffset< XArgType::rank::value >(
                            in_iter.GetOffset(), fInputDimensionSize, &(in_indices[0]));
                        std::size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex< XArgType::rank::value >(
                            fOutputDimensionSize, &(in_indices[0]));
                        (*(out))[out_loc] = *in_iter;
                        ++in_iter;
                    }
                }
                else
                {
                    //zero padding is placed at the start of the array
                    auto in_iter = in->cbegin();
                    auto in_iter_end = in->cend();
                    std::array< std::size_t, XArgType::rank::value > in_index;
                    std::size_t out_index[XArgType::rank::value];
                    while(in_iter != in_iter_end)
                    {
                        //copy the input data to the flipped location in the output array
                        MHO_NDArrayMath::RowMajorIndexFromOffset< XArgType::rank::value >(in_iter.GetOffset(),
                                                                                          in->GetDimensions(), &(in_index[0]));
                        for(std::size_t i = 0; i < XArgType::rank::value; i++)
                        {
                            if(fAxesToXForm[i])
                            {
                                out_index[i] = (fOutputDimensionSize[i] - 1) - in_index[i];
                                //The way were are setting the indexes here when fNormFXMode is enabled
                                //(with n=0 mapping to N/2 is done for compatibility with norm_fx)
                                //TODO...figure out why it is done this way
                                if(fNormFXMode)
                                {
                                    out_index[i] = fOutputDimensionSize[i] - in_index[i];
                                    if(in_index[i] == 0)
                                    {
                                        out_index[i] = fOutputDimensionSize[i] / 2;
                                    }
                                }
                            }
                            else
                            {
                                out_index[i] = in_index[i];
                            }
                        }
                        std::size_t out_loc =
                            MHO_NDArrayMath::OffsetFromRowMajorIndex< XArgType::rank::value >(fOutputDimensionSize, out_index);
                        (*(out))[out_loc] = *in_iter;
                        ++in_iter;
                    }
                }

                IfTableTransformAxis(in, out);
                //successful
                return true;
            }
            else
            {
                //error
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

        XArgType* fTmpWorkspace;
};

} // namespace hops

#endif /*! MHO_EndZeroPadder_H__ */
