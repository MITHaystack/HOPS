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
                fDummy //dummy owner, to keep python from taking ownership of this memory
            };
            return ret_val;
        }

        /** return the N-th axis as a python list object
        *this conversion is NOT copy-free, but we get the same return type for all axis types
        * TODO FIXME -- figure out how we are going to pass interval-labels
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
                msg_fatal("python_bindings", "axis index: "<< index << " exceeds table rank of: "<< fRank << eom );
                std::exit(1);
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
                msg_fatal("python_bindings", "axis index: "<< axis_index << " exceeds table rank of: "<< fRank << eom );
                std::exit(1);
            }
        }

        py::dict GetTags()
        {
            py::dict tags;
            DumpValuesToPyDict<bool>(tags);
            //DumpValuesToPyDict<char>(tags); //don't pass char tags (should we eliminate these?)
            DumpValuesToPyDict<int>(tags);
            DumpValuesToPyDict<double>(tags);
            DumpValuesToPyDict<std::string>(tags);
            return tags;
        }

        void SetTag(std::string key, py::object value)
        {
            //try casting the py::object to the allowed type, and set the tag value 
            //note: we do not allow char-type tags to be set from python
            if(py::isinstance<py::bool_>(value))
            {
                bool val = value.cast<bool>();
                fTable->Insert(key, val);
            } 
            else if(py::isinstance<py::int_>(value))
            {
                int val = value.cast< int >();
                fTable->Insert(key, val);
            } 
            else if(py::isinstance<py::float_>(value))
            {
                double val = value.cast< double >();
                fTable->Insert(key, val);
            } 
            else if(py::isinstance<py::str>(value))
            {
                std::string val = value.cast< std::string >();
                fTable->Insert(key, val);
            } 
            else 
            {
                msg_error("python_bindings", "unable to set tag with key: "<< key <<" since object not castable to bool, int, float, or string "<< eom);
            }

        }

    protected:


        //helper class to act as a python-list filling functor (to return copies)
        class PyListFiller
        {
            public:
                PyListFiller(py::list* alist):fList(alist){};
                ~PyListFiller(){};

                template< typename XAxisType >
                void operator()(const XAxisType& axis)
                {
                    //expect to get some sort of MHO_Axis
                    size_t n = axis.GetSize();
                    for(size_t i=0; i<axis.GetSize(); i++)
                    {
                        fList->append(axis[i]);
                    }
                }

            private:
                py::list* fList;
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
                    size_t n = axis.GetSize();
                    if(fIndex < n)
                    {
                        axis(fIndex) = label_value;
                    }
                    else
                    {
                        //out of bounds error
                        msg_fatal("python_bindings", "error axis coordinate index out of bounds: " << fIndex << " > " << n << "."<< eom );
                        std::exit(1);
                    }
                }

            private:
                std::size_t fIndex;
                py::object* fObject;

        };

        template< typename XDumpType > 
        void DumpValuesToPyDict(py::dict& dump)
        {
            std::vector< std::string > keys;
            auto map = dynamic_cast< MHO_SingleTypeMap< std::string , XDumpType >* >(fTable);
            if(map != nullptr)
            {
                keys = map->DumpKeys();
                for(auto it = keys.begin(); it != keys.end(); it++)
                {
                    XDumpType val;
                    map->Retrieve(*it, val);
                    dump[it->c_str()] = val;
                }
            }
        }


    private:

        MHO_ExtensibleElement* fElement;
        XTableType* fTable;
        unsigned int fRank;
        py::str fDummy;

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
        .def("GetRank", &hops::MHO_PyTableContainer<XTableType>::GetRank)
        .def("GetClassName", &hops::MHO_PyTableContainer<XTableType>::GetClassName)
        .def("GetDimension", &hops::MHO_PyTableContainer<XTableType>::GetDimension)
        .def("GetTags", &hops::MHO_PyTableContainer<XTableType>::GetTags)
        .def("SetTag", &hops::MHO_PyTableContainer<XTableType>::SetTag)
        .def("GetNumpyArray", &hops::MHO_PyTableContainer<XTableType>::GetNumpyArray)
        .def("GetCoordinateAxis", &hops::MHO_PyTableContainer<XTableType>::GetCoordinateAxis)
        .def("SetCoordinateLabel", &hops::MHO_PyTableContainer<XTableType>::SetCoordinateLabel);
}







}//end of hops namespace

#endif /* end of include guard: MHO_PyTableContainer */
