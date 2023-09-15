#ifndef MHO_PyTableContainer_HH__
#define MHO_PyTableContainer_HH__

/*
*File: MHO_PyNDArrayWrapper.hh
*Class: MHO_PyNDArrayWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
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

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
#include <pybind11/stl.h>
namespace py = pybind11;

// #include "MHO_PyAxisContainer.hh"

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
            return ret_val;
        }

    protected:


        //helper class to act as a python-list filling functor
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
        .def("GetNumpyArray", &hops::MHO_PyTableContainer<XTableType>::GetNumpyArray)
        .def("GetCoordinateAxis", &hops::MHO_PyTableContainer<XTableType>::GetCoordinateAxis);
}







}//end of hops namespace

#endif /* end of include guard: MHO_PyNDArrayWrapper */
