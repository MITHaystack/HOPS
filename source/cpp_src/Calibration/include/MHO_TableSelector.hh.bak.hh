#ifndef MHO_TableSelector_HH__
#define MHO_TableSelector_HH__


/*
*File: MHO_TableSelector.hh
*Class: MHO_TableSelector
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

template<class XArgType>
class MHO_TableSelector: public MHO_UnaryOperator< XArgType >
{
    public:
        MHO_TableSelector(){};
        virtual ~MHO_TableSelector(){};

        void Reset(){fAxisSelectionMap.clear();}

        void SelectAxisItems(std::size_t axis_index, const std::vector<std::size_t>& valid_indexes)
        {
            fAxisSelectionMap[axis_index] = valid_indexes;
            //sort to make sure the selected elements are given in increasing order
            std::sort( fAxisSelectionMap[axis_index].begin(), fAxisSelectionMap[axis_index].end() );
        }

    private:

        virtual bool InitializeInPlace(XArgType* in)
        {

        }

        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out)
        {
            std::array<std::size_t, XArgType::rank::value> out_dim;
            in->GetDimensions( &(out_dim[0]) ); //initialize all dimensions to be the same
            for(std::size_t a=0; a < XArgType::rank::value; a++)
            {
                //reduce output dimensions for axes which have selections
                if(fAxisSelectionMap.count(a) == 0)
                {
                    out_dim[a] = fAxisSelectionMap[a].size();
                }
            }
            out->Resize(&(out_dim[0]));
        }

        virtual bool ExecuteInPlace(XArgType* in)
        {
        // 
        //     if(fAxisSelectionMap.size() == 0)
        //     {
        //         //no axis has indices specified, so by default all data is selected, do nothing
        //     }
        //     else if(fAxisSelectionMap.size() == XArgType::rank::value)
        //     {
        //         //all axes have data selected that we need to extract
        //     }
        //     else
        //     {
        //         //some axes have data selected, an some do not (meaning all is passed)
        //         //so we need to sort through the mess 
        //     }
        // }

        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out)
        {
            //assume for now that out is appropriately sized
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
                    MHO_NDArrayMath::RowMajorIndexFromOffset(i, &(out_dim[0]), &(out_loc[0]) );

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
                    MHO_NDArrayMath::RowMajorIndexFromOffset(i, &(out_dim[0]), &(out_loc[0]) );

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

        }



        //data
        bool fInitialized;

        //map which holds the indexes selected from each axis
        //if no item is present for a particular axis, the assumption is that all pre-existing  
        //elements are selected/passed in the selection process
        std::map< std::size_t, std::vector< std::size_t > > fAxisSelectionMap;

        //workspace
        XArrayType fWorkspace;



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//TAKEN FROM SUBSAMPLE OP -- ADAPT

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




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////





















};

}//end hops namespace

#endif /* end of include guard: MHO_TableSelector_HH__ */
