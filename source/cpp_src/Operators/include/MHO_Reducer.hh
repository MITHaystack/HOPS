#ifndef MHO_Reducer_HH__
#define MHO_Reducer_HH__

#include <algorithm>

#include "MHO_CompoundReductions.hh" //for operator type definitions
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
*@file MHO_Reducer.hh
*@class MHO_Reducer
*@author J. Barrett - barrettj@mit.edu
*@date Tue Nov 17 16:53:15 2020 -0500
*@brief Reduce a multi-dimensional array via a templated operation
*(e.g. summation) along the (runtime) specified dimensions. The output array has the same
dimensionality (i.e. XArrayType::rank::value). However, the axes over which reduction occured
will have a size of 1. The output array will be resized if/as needed.
*/

/**
 * @brief Class MHO_Reducer
 */
template< typename XArrayType, template< typename > class XFunctorType >
class MHO_Reducer: public MHO_UnaryOperator< XArrayType >
{
    public:
        using XItemType = typename XArrayType::value_type;

        MHO_Reducer()
        {
            fInitialized = false;
            for(std::size_t i = 0; i < XArrayType::rank::value; i++)
            {
                fAxesToReduce[i] = 0;
            }
        }

        virtual ~MHO_Reducer(){};

        /**
         * @brief Sets axis index for reduction operation
         * @details set the indices of the axes over which we run the reduction.
         * This must be set before we can initialize/execute
         * for example for a 3D array, if we wanted to reduce along the
         * last axis only we would call this->ReduceAxis(2), or alternatively if we
         * wanted to reduce along both the first and last axis
         * we would call this->ReduceAxis(0), this->ReduceAxis(2)
         * @param axis_index Index of the axis to reduce.
         */
        void ReduceAxis(std::size_t axis_index)
        {
            fInitialized = false;
            if(axis_index < XArrayType::rank::value)
            {
                fAxesToReduce[axis_index] = 1;
            }
            else
            {
                msg_error("operators", "Cannot reduce axis with index: " << axis_index << "for array with rank: "
                                                                         << XArrayType::rank::value << eom);
            }
        }

        //de-select all axes
        /**
         * @brief De-selects all axes by setting internal flags and arrays to default values (no reduction)
         */
        void ClearAxisSelection()
        {
            fInitialized = false;
            for(std::size_t i = 0; i < XArrayType::rank::value; i++)
            {
                fAxesToReduce[i] = 0;
            }
        }

    protected:
        /**
         * @brief Initializes XArrayType in-place by calling InitializeOutOfPlace with given workspace.
         *
         * @param in Pointer to XArrayType object for initialization
         * @return Boolean indicating success of initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArrayType* in) override { return InitializeOutOfPlace(in, &fWorkspace); }

        /**
         * @brief Executes operation in-place by calling ExecuteOutOfPlace and copying result back to input
         *
         * @param in Input array of type XArrayType*
         * @return Status of execution as boolean
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            bool status = ExecuteOutOfPlace(in, &fWorkspace);
            //"in-place" execution requires a copy from the workspace back to the object we are modifying
            in->Copy(fWorkspace);
            return status;
        }

        /**
         * @brief Initializes out-of-place operation using input array and workspace.
         *
         * @param in Input array of type XArrayType
         * @param out Output array of type XArrayType
         * @return True if initialization is successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                std::size_t in_dim[XArrayType::rank::value];
                std::size_t out_dim[XArrayType::rank::value];
                std::size_t current_out_dim[XArrayType::rank::value];
                in->GetDimensions(in_dim);
                out->GetDimensions(current_out_dim);

                //first figure out what the remaining dim sizes are to be
                //after the array is contracted along the specified dimensions
                for(std::size_t i = 0; i < XArrayType::rank::value; i++)
                {
                    if(fAxesToReduce[i])
                    {
                        out_dim[i] = 1;
                    }
                    else
                    {
                        out_dim[i] = in_dim[i];
                    }
                }

                bool have_to_resize = false;
                for(std::size_t i = 0; i < XArrayType::rank::value; i++)
                {
                    if(out_dim[i] != current_out_dim[i])
                    {
                        have_to_resize = true;
                        break;
                    }
                }

                if(have_to_resize)
                {
                    out->Resize(out_dim);
                }

                //must set the entire output array to the identity of the currently
                //templated
                out->SetArray(this->fReductionFunctor.identity);
                fInitialized = true;
            }
            return fInitialized;
        }

        /**
         * @brief Function ExecuteOutOfPlace - carries out the array reduction
         *
         * @param in (const XArrayType*)
         * @param out (XArrayType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            if(fInitialized)
            {
                std::size_t in_dim[XArrayType::rank::value];
                std::size_t out_dim[XArrayType::rank::value];
                in->GetDimensions(in_dim);
                out->GetDimensions(out_dim);

                std::size_t offset;
                std::size_t in_loc[XArrayType::rank::value];
                std::size_t out_loc[XArrayType::rank::value];
                auto iter_begin = in->cbegin();
                auto iter_end = in->cend();
                for(auto iter = iter_begin; iter != iter_end; ++iter)
                {
                    //get the input indices for each dimension
                    offset = iter.GetOffset();
                    MHO_NDArrayMath::RowMajorIndexFromOffset< XArrayType::rank::value >(offset, in->GetDimensions(),
                                                                                        &(in_loc[0]));

                    //set the output indices to collapse each index of the dimensions under reduction
                    for(std::size_t i = 0; i < XArrayType::rank::value; i++)
                    {
                        out_loc[i] = std::min(in_loc[i], out_dim[i] - 1);
                    }
                    //find offset to location in output array
                    std::size_t m = MHO_NDArrayMath::OffsetFromRowMajorIndex< XArrayType::rank::value >(out_dim, out_loc);
                    //execute the reduction operator +=  or *= or user-defined
                    fReductionFunctor((*(out))[m], *iter);
                }

                for(std::size_t i = 0; i < XArrayType::rank::value; i++)
                {
                    // if(fAxesToReduce[i] == 1){IfTableReduceAxis(in, out, i);}
                    IfTableReduceAxis(in, out, i);
                }

                return true;
            }
            return false;
        }

    private:
        /**
         * @brief Class AxisReducer - handles special treament of axes objects for a MHO_TableContainer input
         */
        class AxisReducer
        {
            public:
                AxisReducer(std::size_t reduce_ax): fReduce(false)
                {
                    if(reduce_ax)
                    {
                        fReduce = true;
                    }
                };

                ~AxisReducer(){};

                template< typename XAxisType > void operator()(const XAxisType& axis1, XAxisType& axis2)
                {
                    if(fReduce)
                    {
                        //all we do is set the axis label to the start value of the
                        //original axis, perhaps we ought to enable an option to use
                        //the mean value as well/instead?
                        auto it1 = axis1.cbegin();
                        auto it2 = axis2.begin();
                        *it2 = *it1;
                        axis2.CopyTags(axis1); //copy the axis tags
                    }
                    else
                    {
                        //copy the axis
                        axis2.Copy(axis1);
                    }
                }

            private:
                bool fReduce;
        };

        //default...does nothing
        /**
         * @brief Reduces a specified axis in an input XArrayType and stores the result in output.
         *
         * @tparam XCheckType Template parameter XCheckType
         * @param !in Input XArrayType to reduce
         * @param !out Output XArrayType after reduction
         * @param !ax_index Index of the axis to reduce
         */
        template< typename XCheckType = XArrayType >
        typename std::enable_if< !std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableReduceAxis(const XArrayType* /*!in*/, XArrayType* /*!out*/, std::size_t /*!ax_index)*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        /**
         * @brief Reduces a specified axis in an input XArrayType and stores result in output.
         *
         * @tparam XCheckType Template parameter XCheckType
         * @param in Input XArrayType to reduce
         * @param out Output XArrayType for reduced data
         * @param ax_index Index of the axis to reduce
         */
        template< typename XCheckType = XArrayType >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableReduceAxis(const XArrayType* in, XArrayType* out, std::size_t ax_index)
        {
            AxisReducer axis_reducer(fAxesToReduce[ax_index]);
            apply_at2< typename XArrayType::axis_pack_tuple_type, AxisReducer >(*in, *out, ax_index, axis_reducer);
            if(ax_index == 0)
            {
                out->CopyTags(*in);
            } //make sure the table tags get copied, just once
        }

        bool fInitialized;
        std::size_t fAxesToReduce[XArrayType::rank::value];
        XFunctorType< XItemType > fReductionFunctor;
        XArrayType fWorkspace;
};

} // namespace hops

#endif /*! MHO_Reducer_H__ */
