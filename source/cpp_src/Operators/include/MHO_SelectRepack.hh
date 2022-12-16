#ifndef MHO_SelectRepack_HH__
#define MHO_SelectRepack_HH__


/*
*File: MHO_SelectRepack.hh
*Class: MHO_SelectRepack
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Thu 15 Dec 2022 02:42:45 PM EST
*Description:
* operator to select data from table and repack it into an entirely new table,
* this typically would involve lots of copying (expensive), so it should be used
* sparringly (e.g. initial or final data selection)

*/

#include <map>
#include <vector>

#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"

#include "MHO_ContainerDefinitions.hh"

namespace hops
{

template< class XArgType>
class MHO_SelectRepack:
    public MHO_UnaryOperator< XArgType >
{
    public:

        MHO_SelectRepack()
        {
            fInitialized = false;
            fAxisSelectionMap.clear();
        };

        virtual ~MHO_SelectRepack(){};

        void Reset(){fAxisSelectionMap.clear();}

        void SelectAxisItems(std::size_t axis_index, const std::vector<std::size_t>& valid_indexes)
        {
            fAxisSelectionMap[axis_index] = valid_indexes;
            //sort to make sure the selected elements are given in increasing order
            std::sort( fAxisSelectionMap[axis_index].begin(), fAxisSelectionMap[axis_index].end() );
            fInitialized = false;
        }

    protected:

        virtual bool InitializeInPlace(XArgType* in) override
        {
            return InitializeOutOfPlace(in, &fWorkspace);
        }

        virtual bool ExecuteInPlace(XArgType* in) override
        {
            bool status = ExecuteOutOfPlace(in, &fWorkspace);
            //"in-place" execution requires a copy from the workspace back to the object we are modifying
            in->Copy(fWorkspace);
            return status;
        }

        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                std::array<std::size_t, XArgType::rank::value> out_dim = DetermineOutputDimensions(in);
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

        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if(fInitialized)
            {
                std::array<std::size_t, XArgType::rank::value> out_dim;
                std::array<std::size_t, XArgType::rank::value> out_loc;
                std::array<std::size_t, XArgType::rank::value> in_loc;

                out->GetDimensions( &(out_dim[0]) );
                std::size_t total_size = out->GetSize();

                if(fAxisSelectionMap.size() == 0)
                {
                    //no axis has indices specified, so by default all data is selected
                    out->Copy(*in);
                }
                else if(fAxisSelectionMap.size() == XArgType::rank::value)
                {
                    //all axes have data selected that we need to extract
                    for(std::size_t i=0; i<total_size; i++)
                    {
                        //compute the indexes into the 'out' array 
                        MHO_NDArrayMath::RowMajorIndexFromOffset<XArgType::rank::value>(i, &(out_dim[0]), &(out_loc[0]) );

                        //compute the indexes into the 'in' array, remapping where needed
                        //from the selections
                        for(std::size_t a=0; a < XArgType::rank::value; a++)
                        {
                            in_loc[a] = (fAxisSelectionMap[a])[out_loc[a]];
                            out->ValueAt(out_loc) = in->ValueAt(in_loc);
                        }
                    }
                }
                else
                {
                    //some axes have data selected, an some do not (meaning all is passed)
                    //so we need to sort through the mess 
                    for(std::size_t i=0; i<total_size; i++)
                    {
                        //compute the indexes into the 'out' array 
                        MHO_NDArrayMath::RowMajorIndexFromOffset<XArgType::rank::value>(i, &(out_dim[0]), &(out_loc[0]) );

                        //compute the indexes into the 'in' array, remapping where needed
                        //from the selections
                        for(std::size_t a=0; a < XArgType::rank::value; a++)
                        {
                            if(fAxisSelectionMap.count(a) == 0)
                            {
                                in_loc[a] = out_loc[a];
                            }
                            else 
                            {
                                in_loc[a] = (fAxisSelectionMap[a])[out_loc[a]];
                            }
                            out->ValueAt(out_loc) = in->ValueAt(in_loc);
                        }
                    }
                }

                IfTableSelectOnAxes(in,out);

                return true;
            }
            else{ return false;}
        }




    private:

        std::array<std::size_t, XArgType::rank::value> DetermineOutputDimensions(const XArgType* in)
        {
            std::array<std::size_t, XArgType::rank::value> out_dim;
            in->GetDimensions( &(out_dim[0]) ); //initialize all dimensions to be the same as input
            for(std::size_t a=0; a < XArgType::rank::value; a++)
            {
                //reduce output dimensions for those axes which have selections
                if(fAxisSelectionMap.count(a) == 0)
                {
                    out_dim[a] = fAxisSelectionMap[a].size();
                }
            }
            return out_dim;
        }
        
        void ConditionallyResizeOutput(const std::array<std::size_t, XArgType::rank::value>& dims, XArgType* out)
        {
            auto out_dim = out->GetDimensionArray();
            bool have_to_resize = false;
            for(std::size_t i=0; i<XArgType::rank::value; i++)
            {
                if(out_dim[i] != dims[i] ){have_to_resize = true;}
            }
            if(have_to_resize){ out->Resize( &(dims[0]) );}
        }

        //default...does nothing
        template< typename XCheckType = XArgType >
        typename std::enable_if< !std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableSelectOnAxes(const XArgType* /*in*/, XArgType* /*out*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        template< typename XCheckType = XArgType >
        typename std::enable_if< std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableSelectOnAxes(const XArgType* in, XArgType* out)
        {
            for(std::size_t a=0; a<XArgType::rank::value; a++)
            {
                SelectOnAxis axis_sub_sampler( fAxisSelectionMap[a] );
                apply_at2< typename XArgType::axis_pack_tuple_type, SelectOnAxis >( *in, *out, a, axis_sub_sampler);
            }
        }


        class SelectOnAxis
        {
            public:
                SelectOnAxis(std::vector< std::size_t > selection):
                    fSelection(selection)
                {};
                ~SelectOnAxis(){};

                template< typename XAxisType >
                void operator()(const XAxisType& axis1, XAxisType& axis2)
                {
                    for(std::size_t i=0; i<fSelection.size();i++)
                    {
                        axis2(i) = axis1( fSelection[i] );
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







};


#endif /* end of include guard: MHO_SelectRepack_HH__ */
