#ifndef MHO_PyAxisContainer_HH__
#define MHO_PyAxisContainer_HH__

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

namespace hops
{

/**extension which allows us to present the contents of the XAxisType to
* python as a numpy array + python list objects for the axes.
* Changes made to the contents of numpy array will be exposed to the c++ side.
* However, no changes to the size/shape or axes of the array are supported.
*/

template< typename XAxisType >
class MHO_PyAxisContainer
{
    public:

        MHO_PyAxisContainer(MHO_ExtensibleElement* element):
            fElement(element)
        {
            fRank = XAxisType::rank::value;
            fTable = dynamic_cast< XAxisType* >(element);
        };


        virtual ~MHO_PyAxisContainer(){};


        /**return the ND-array data block as a numpy array
        *this transfer is copy-free
        */
        py::array_t< typename XAxisType::value_type > GetNumpyArray()
        {
            auto strides = fTable->GetStrideArray();
            for(std::size_t i=0; i<fRank; i++){strides[i] *= sizeof(typename XAxisType::value_type);}
            py::array_t< typename XAxisType::value_type > ret_val
            {
                fTable->GetDimensionArray(),
                strides,
                fTable->GetData(),
                fDummy //dummy owner, to keep python from taking ownership of this memory
            };
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
        XAxisType* fTable;
        unsigned int fRank;
        py::str fDummy;


};


//XAxisType must inherit from MHO_NDArrayWrapper<XValueType, RANK>
template< typename XAxisType >
void
DeclarePyTableContainer(py::module &m, std::string pyclass_name = "")
{
    if(pyclass_name == "")
    {
        pyclass_name = MHO_ClassName< XAxisType >();
    }

    py::class_< MHO_PyAxisContainer<XAxisType> >(m, pyclass_name.c_str() )
        //no __init__ def here, as this class is not meant to be constructable on the python side
        .def("GetNumpyArray", &hops::MHO_PyAxisContainer<XAxisType>::GetNumpyArray)
        .def("GetCoordinateAxis", &hops::MHO_PyAxisContainer<XAxisType>::GetCoordinateAxis);
}







}//end of hops namespace

#endif /* end of include guard: MHO_PyNDArrayWrapper */
