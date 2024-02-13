#ifndef MHO_PyTableContainer_HH__
#define MHO_PyTableContainer_HH__

/*
*File: MHO_PyTableContainer.hh
*Class: MHO_PyTableContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Fri Sep 15 10:03:38 PM EDT 2023
*Description:
*/

#include <complex>
#include <string>
#include <sstream>
#include <array>
#include <tuple>
#include <type_traits>

#include "MHO_Meta.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ExtensibleElement.hh"
#include "MHO_TemplateTypenameDeduction.hh"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
#include <pybind11/stl.h>
namespace py = pybind11;

// #include "MHO_PyAxisHelpers.hh"

namespace hops
{

/**extension which allows us to present the contents of the XTableType to
* python as a numpy array + python list objects for the axes.
* Changes made to the contents of numpy array will be exposed to the c++ side.
* However, no changes to the size/shape or axes of the array are supported.
*/


template< typename XTableType >
class MHO_PyTableContainer
{
    public:

        MHO_PyTableContainer(MHO_ExtensibleElement* element):
            fElement(element)
        {
            fRank = XTableType::rank::value;
            fTable = dynamic_cast< XTableType* >(element);
        };


        virtual ~MHO_PyTableContainer(){};

        std::size_t GetRank() const {return fTable->GetRank();}

        std::size_t GetDimension( std::size_t index ) const {return fTable->GetDimension(index); }

        std::string GetClassName() const
        {
            return MHO_ClassName< XTableType >();
        }

        /**return the ND-array data block as a numpy array
        *this transfer is copy-free
        */
        py::array_t< typename XTableType::value_type > GetNumpyArray()
        {
            auto strides = fTable->GetStrideArray();
            for(std::size_t i=0; i<fRank; i++){strides[i] *= sizeof(typename XTableType::value_type);}
            py::array_t< typename XTableType::value_type > ret_val
            {
                fTable->GetDimensionArray(),
                strides,
                fTable->GetData(),
                py::str(fDummy) //dummy owner, to keep python from taking ownership of this memory
            };
            return ret_val;
        }

        /** return the N-th axis as a python list object
        *this conversion is NOT copy-free, but we get the same return type for all axis types
        */
        py::list GetCoordinateAxis(size_t index)
        {
            py::list ret_val;
            if(index < fRank)
            {
                PyListFiller filler(&ret_val);
                apply_at< typename XTableType::axis_pack_tuple_type, PyListFiller >( *fTable, index, filler);
            }
            else
            {
                msg_error("python_bindings", "axis index: "<< index << " exceeds table rank of: "<< fRank << eom );
                std::stringstream ss;
                ss << "axis index: "<< index << " exceeds table rank of: "<< fRank;
                py::print("error: ", ss.str());
            }
            return ret_val;
        }

        //super crude way to modify the coordinate
        void SetCoordinateLabel(std::size_t axis_index, std::size_t label_index, py::object label)
        {
            if(axis_index < fRank)
            {
                PyAxisLabelModifier modifier(label_index, &label);
                apply_at< typename XTableType::axis_pack_tuple_type, PyAxisLabelModifier >( *fTable, axis_index, modifier);
            }
            else
            {
                msg_error("python_bindings", "axis index: "<< axis_index << " exceeds table rank of: "<< fRank << eom );
                std::stringstream ss;
                ss << "axis index: "<< axis_index << " exceeds table rank of: "<< fRank;
                py::print("error: ", ss.str());
            }
        }

        py::dict GetMetaData()
        {
            return GetTableTags< XTableType >(fTable);
        }

        void SetMetaData(py::dict metadata)
        {
            return SetTableTags< XTableType >(fTable, metadata);
        }

        py::dict GetCoordinateAxisMetaData(std::size_t index)
        {
            py::dict ret_val;
            if(index < fRank)
            {
                PyAxisMetaDataFiller filler(&ret_val);
                apply_at< typename XTableType::axis_pack_tuple_type, PyAxisMetaDataFiller >(*fTable, index, filler);
            }
            else
            {
                msg_error("python_bindings", "axis index: "<< index << " exceeds table rank of: "<< fRank << eom );
                std::stringstream ss;
                ss << "axis index: "<< index << " exceeds table rank of: "<< fRank;
                py::print("error: ", ss.str());
            }
            return ret_val;
        }
        
        //whole-sale re-setting of metadata, this may be over-kill and actually a bit dangerous
        //since the index/interval labels are stored in the metadata object, if we are not 
        //passing an object which is derived from an already retrieved copy we may lose info
        void SetCoordinateAxisMetaData(std::size_t index, py::dict metadata)
        {
            if(index < fRank)
            {
                PyAxisMetaDataSetter filler(&metadata);
                apply_at< typename XTableType::axis_pack_tuple_type, PyAxisMetaDataSetter >(*fTable, index, filler);
            }
            else
            {
                msg_error("python_bindings", "axis index: "<< index << " exceeds table rank of: "<< fRank << eom );
                std::stringstream ss;
                ss << "axis index: "<< index << " exceeds table rank of: "<< fRank;
                py::print("error: ", ss.str());
            }
        }

    private:

        template< typename XDataTableType >
        static py::dict GetTableTags(XDataTableType* table)
        {
            py::dict tags = table->GetMetaDataAsJSON();
            return tags;
        }
        
        template< typename XDataTableType >
        static void SetTableTags(XDataTableType* table, py::dict metadata)
        {
            mho_json md = metadata;
            table->SetMetaDataAsJSON(md);
        }

        //helper class to act as a python-list filling functor (to return copies)
        class PyListFiller
        {
            public:
                PyListFiller(py::list* alist):fList(alist){};
                ~PyListFiller(){};

                template< typename XAxisType >
                void operator()(const XAxisType& axis)
                {
                    for(size_t i=0; i<axis.GetSize(); i++){fList->append(axis[i]);}
                }

            private:
                py::list* fList;
        };
        
        //helper class to act as a python dict filling functor (to return copies of meta data)
        class PyAxisMetaDataFiller
        {
            public:
                PyAxisMetaDataFiller(py::dict* adict):fDict(adict){};
                ~PyAxisMetaDataFiller(){};

                template< typename XAxisType >
                void operator()(const XAxisType& axis)
                {
                    *fDict = axis.GetMetaDataAsJSON();
                }

            private:
                py::dict* fDict;
        };
        
        //helper class to act as a python dict filling functor (to return copies of meta data)
        class PyAxisMetaDataSetter
        {
            public:
                PyAxisMetaDataSetter(py::dict* adict):fDict(adict){};
                ~PyAxisMetaDataSetter(){};

                template< typename XAxisType >
                void operator()(XAxisType& axis)
                {
                    axis.SetMetaDataAsJSON( *fDict );
                }

            private:
                py::dict* fDict;
        };


        //helper class that allows us to set the value of an axis label from python
        class PyAxisLabelModifier
        {
            public:

                //constructor accepts an coordinate index and a python object
                //and will attempt to case the object to the underyling axis-label type
                //and assign it at the location specified by the index
                PyAxisLabelModifier(std::size_t index, py::object* label_object):
                    fIndex(index),
                    fObject(label_object)
                {};
                ~PyAxisLabelModifier(){};

                template< typename XAxisType >
                void operator()(XAxisType& axis)
                {
                    typename XAxisType::value_type label_value;
                    label_value = fObject->cast< typename XAxisType::value_type >();

                    //expect to get some sort of MHO_Axis
                    if(fIndex < axis.GetSize() ){ axis(fIndex) = label_value; }
                    else
                    {
                        msg_error("python_bindings", "error axis coordinate index out of bounds: " << fIndex << " > " << axis.GetSize() << "."<< eom );
                        std::stringstream ss;
                        ss << "axis coordinate index out of bounds: " << fIndex  << " > " << axis.GetSize();
                        py::print("error: ", ss.str());
                    }
                }

            private:
                std::size_t fIndex;
                py::object* fObject;

        };


    private:

        MHO_ExtensibleElement* fElement;
        XTableType* fTable;
        unsigned int fRank;
        std::string fDummy;
};


//XTableType must inherit from MHO_NDArrayWrapper<XValueType, RANK>
template< typename XTableType >
void
DeclarePyTableContainer(py::module &m, std::string pyclass_name = "")
{
    if(pyclass_name == "")
    {
        pyclass_name = MHO_ClassName< XTableType >();
    }

    py::class_< MHO_PyTableContainer<XTableType> >(m, pyclass_name.c_str() )
        //no __init__ def here, as this class is not meant to be constructable on the python side
        .def("get_rank", &hops::MHO_PyTableContainer<XTableType>::GetRank)
        .def("get_classname", &hops::MHO_PyTableContainer<XTableType>::GetClassName)
        .def("get_dimension", &hops::MHO_PyTableContainer<XTableType>::GetDimension)
        .def("get_metadata", &hops::MHO_PyTableContainer<XTableType>::GetMetaData)
        .def("set_metadata", &hops::MHO_PyTableContainer<XTableType>::SetMetaData)
        .def("get_numpy_array", &hops::MHO_PyTableContainer<XTableType>::GetNumpyArray)
        .def("get_axis", &hops::MHO_PyTableContainer<XTableType>::GetCoordinateAxis)
        .def("get_axis_metadata", &hops::MHO_PyTableContainer<XTableType>::GetCoordinateAxisMetaData)
        .def("set_axis_metadata", &hops::MHO_PyTableContainer<XTableType>::SetCoordinateAxisMetaData)
        .def("set_axis_label", &hops::MHO_PyTableContainer<XTableType>::SetCoordinateLabel);
}







}//end of hops namespace

#endif /* end of include guard: MHO_PyTableContainer */
