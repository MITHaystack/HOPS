#ifndef MHO_SelectRepack_HH__
#define MHO_SelectRepack_HH__

#include <map>
#include <vector>

#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"

#include "MHO_ContainerDefinitions.hh"

namespace hops
{

/*!
 *@file MHO_SelectRepack.hh
 *@class MHO_SelectRepack
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Dec 15 16:45:22 2022 -0500
 *@brief
 * operator to select data from table and repack it into an entirely new table,
 * this typically would involve lots of copying (expensive), so it should be used
 * sparringly (e.g. initial or final coarse data selection)
 */

/**
 * @brief Class MHO_SelectRepack
 */
template< class XArgType > class MHO_SelectRepack: public MHO_UnaryOperator< XArgType >
{
    public:
        MHO_SelectRepack()
        {
            fInitialized = false;
            fAxisSelectionMap.clear();
        };

        virtual ~MHO_SelectRepack(){};

        /**
         * @brief Clears all entries from fAxisSelectionMap.
         */
        void Reset() { fAxisSelectionMap.clear(); }

        /**
         * @brief Stores valid indexes for a given axis and marks selection as uninitialized.
         *
         * @param axis_index Index of the axis to store valid indexes for.
         * @param valid_indexes Vector of valid indexes for the specified axis.
         */
        void SelectAxisItems(std::size_t axis_index, const std::vector< std::size_t >& valid_indexes)
        {
            fAxisSelectionMap[axis_index] = valid_indexes;
            //sort to make sure the selected elements are given in increasing order
            std::sort(fAxisSelectionMap[axis_index].begin(), fAxisSelectionMap[axis_index].end());
            fInitialized = false;
        }

    protected:
        /**
         * @brief Initializes in-place by calling InitializeOutOfPlace with workspace.
         *
         * @param in Input pointer to XArgType object
         * @return Boolean indicating success of initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArgType* in) override { return InitializeOutOfPlace(in, &fWorkspace); }

        /**
         * @brief Executes operation in-place by calling ExecuteOutOfPlace and copying result back to input.
         *
         * @param in Input object of type XArgType* that will be modified in-place
         * @return Status of the execution operation
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArgType* in) override
        {
            bool status = ExecuteOutOfPlace(in, &fWorkspace);
            //"in-place" execution requires a copy from the workspace back to the object we are modifying
            in->Copy(fWorkspace);
            return status;
        }

        /**
         * @brief Initializes out-of-place processing for given input and output arguments.
         *
         * @param in Const reference to input argument of type XArgType
         * @param out Reference to output argument of type XArgType
         * @return Boolean indicating success or failure of initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                std::array< std::size_t, XArgType::rank::value > out_dim = DetermineOutputDimensions(in);
                ConditionallyResizeOutput(out_dim, out);
                fInitialized = true;
                return true;
            }
            else
            {
                fInitialized = false;
                return false;
            }
        }

        /**
         * @brief Function ExecuteOutOfPlace
         *
         * @param in (const XArgType*)
         * @param out (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if(fInitialized)
            {
                std::array< std::size_t, XArgType::rank::value > out_dim;
                std::array< std::size_t, XArgType::rank::value > out_loc;
                std::array< std::size_t, XArgType::rank::value > in_loc;

                out->GetDimensions(&(out_dim[0]));
                std::size_t total_size = out->GetSize();

                if(fAxisSelectionMap.size() == 0)
                {
                    //no axis has indices specified, so by default all data is selected
                    out->Copy(*in);
                }
                else if(fAxisSelectionMap.size() == XArgType::rank::value)
                {
                    //all axes have data selected that we need to extract
                    for(std::size_t i = 0; i < total_size; i++)
                    {
                        //compute the indexes into the 'out' array
                        MHO_NDArrayMath::RowMajorIndexFromOffset< XArgType::rank::value >(i, &(out_dim[0]), &(out_loc[0]));

                        //compute the indexes into the 'in' array, remapping where needed
                        //from the selections
                        for(std::size_t a = 0; a < XArgType::rank::value; a++)
                        {
                            in_loc[a] = (fAxisSelectionMap[a])[out_loc[a]];
                            out->ValueAt(out_loc) = in->ValueAt(in_loc);
                        }
                    }
                    IfTableSelectOnAxes(in, out);
                }
                else
                {
                    //some axes have data selected, and some do not (meaning all is passed)
                    //so we need to sort through the mess
                    for(std::size_t i = 0; i < total_size; i++)
                    {
                        //compute the indexes into the 'out' array
                        MHO_NDArrayMath::RowMajorIndexFromOffset< XArgType::rank::value >(i, &(out_dim[0]), &(out_loc[0]));

                        //compute the indexes into the 'in' array, remapping where needed
                        //from the selections
                        for(std::size_t a = 0; a < XArgType::rank::value; a++)
                        {
                            if(fAxisSelectionMap.count(a) == 0)
                            {
                                in_loc[a] = out_loc[a];
                            }
                            else
                            {
                                in_loc[a] = (fAxisSelectionMap[a])[out_loc[a]];
                            }
                        }
                        out->ValueAt(out_loc) = in->ValueAt(in_loc);
                    }
                }

                IfTableSelectOnAxes(in, out);

                return true;
            }
            else
            {
                return false;
            }
        }

    private:
        /**
         * @brief Determines output dimensions based on input dimensions and axis selections.
         *
         * @param in Input argument type pointer.
         * @return Output dimensions as an array of size_t.
         */
        std::array< std::size_t, XArgType::rank::value > DetermineOutputDimensions(const XArgType* in)
        {
            std::array< std::size_t, XArgType::rank::value > out_dim;
            in->GetDimensions(&(out_dim[0])); //initialize all dimensions to be the same as input
            for(std::size_t a = 0; a < XArgType::rank::value; a++)
            {
                //reduce output dimensions for those axes which have selections
                if(fAxisSelectionMap.count(a) != 0)
                {
                    out_dim[a] = fAxisSelectionMap[a].size();
                }
            }
            return out_dim;
        }

        /**
         * @brief Checks and conditionally resizes output dimensions if they differ from input.
         *
         * @param dims Input dimension array to compare with output.
         * @param out (XArgType*)
         */
        void ConditionallyResizeOutput(const std::array< std::size_t, XArgType::rank::value >& dims, XArgType* out)
        {
            auto out_dim = out->GetDimensionArray();
            bool have_to_resize = false;
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
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
         * @brief Applies table selection on axes for input XArgType and outputs result to another XArgType.
         *
         * @tparam XCheckType Template parameter XCheckType
         * @param !in Parameter description
         * @param !out Parameter description
         */
        template< typename XCheckType = XArgType >
        typename std::enable_if< !std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableSelectOnAxes(const XArgType* /*!in*/, XArgType* /*!out*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        /**
         * @brief Applies selection on axes for input XArgType and outputs result to another XArgType.
         *
         * @tparam XCheckType Template parameter XCheckType
         * @param in Input XArgType data for selection
         * @param out Output XArgType after applying selection
         */
        template< typename XCheckType = XArgType >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableSelectOnAxes(const XArgType* in, XArgType* out)
        {
            for(std::size_t a = 0; a < XArgType::rank::value; a++) //apply to all axes
            {
                SelectOnAxis axis_sub_sampler(fAxisSelectionMap[a]);
                apply_at2< typename XArgType::axis_pack_tuple_type, SelectOnAxis >(*in, *out, a, axis_sub_sampler);
            }
            out->CopyTags(*in); //make sure the table tags get copied
        }

        /**
         * @brief Class SelectOnAxis
         */
        class SelectOnAxis
        {
            public:
                SelectOnAxis(std::vector< std::size_t > selection): fSelection(selection){};
                ~SelectOnAxis(){};

                template< typename XAxisType > void operator()(const XAxisType& axis1, XAxisType& axis2)
                {
                    if(fSelection.size() != 0)
                    {
                        //copy all of the axis meta data
                        axis2.CopyTags(axis1);
                        //then remove the index labels
                        axis2.ClearIndexLabels();
                        //now copy back the selected index labels
                        for(std::size_t i = 0; i < fSelection.size(); i++)
                        {
                            axis2(i) = axis1(fSelection[i]);
                            mho_json obj = axis1.GetLabelObject(fSelection[i]);
                            axis2.SetLabelObject(obj, i);
                        }

                        TODO_FIXME_MSG("TODO FIXME -- ensure that only the proper interval tags are selected/copied here.")
                        //axis2.ClearIntervalLabels();
                        //it doesn't make sense to copy the original interval labels since we have changed the organization of this axis
                        //what ought to do is figure out the overlap between previous interval labels and items of the new axis
                        //and create new overlapping intervals with the appropriate tags.
                    }
                    else
                    {
                        axis2.Copy(axis1); //no selection done on this axis, just copy everything
                    }
                }

            private:
                std::vector< std::size_t > fSelection;
        };

        bool fInitialized;
        XArgType fWorkspace;

        //map which holds the indexes selected from each axis
        //if no item is present for a particular axis, the assumption is that all pre-existing
        //elements are selected/passed in the selection process
        std::map< std::size_t, std::vector< std::size_t > > fAxisSelectionMap;
};

}; // namespace hops

#endif /*! end of include guard: MHO_SelectRepack_HH__ */
