#ifndef MHO_PyNDArrayWrapper_HH__
#define MHO_PyNDArrayWrapper_HH__

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

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
namespace py = pybind11;


#include "MHO_NDArrayWrapper.hh"
#include "MHO_ExtensibleElement.hh"
#include "MHO_TemplateTypenameDeduction.hh"
#include "MHO_ChannelizedVisibilities.hh"

namespace hops
{


template< typename XArrayType >
class MHO_PyDArrayWrapper
{
    public:

        MHO_PyDArrayWrapper(MHO_ExtensibleElement* element):
            fElement(element)
        {
            fRank = XArrayType::rank::value;
            fNDArray = dynamic_cast< XArrayType* >(element);
        };


        virtual ~MHO_PyDArrayWrapper()
        {

        };

        py::array_t< typename XArrayType::value_type > GetAsPyArray()
        {
            auto strides = fNDArray->GetStrideArray();
            for(std::size_t i=0; i<fRank; i++){strides[i] *= sizeof(typename XArrayType::value_type);}
            py::array_t< typename XArrayType::value_type > ret_val
            {
                fNDArray->GetDimensionArray(),
                strides,
                fNDArray->GetData(),
                fDummy //dummy owner, to keep python from taking ownership of this memory
            };
            return ret_val;
        }


    protected:

        MHO_ExtensibleElement* fElement;
        XArrayType* fNDArray;
        unsigned int fRank;
        py::str fDummy;


};




//
// //XArrayType must inherit from MHO_NDArrayWrapper<XValueType, RANK>
// template< typename XArrayType >
// void
// DeclarePyNDArrayWrapper(py::module &m, std::string pyclass_name = "")
// {
//     if(pyclass_name == "")
//     {
//         pyclass_name = MHO_ClassName< XArrayType >();
//     }
//     using XValueType = typename XArrayType::value_type;
//
//     py::class_<XArrayType>(m, pyclass_name.c_str(), py::buffer_protocol())
//         .def_buffer([](XArrayType& obj) -> py::buffer_info
//         {
//             return py::buffer_info(
//                 //Raw pointer to buffer data
//                 obj.GetData(),
//                 //size of one scalar element of the array
//                 sizeof( XValueType ),
//                 // Python struct-style format descriptor
//                 py::format_descriptor< XValueType >::format(),
//                 // Number of dimensions
//                 XArrayType::rank::value,
//                 // Buffer dimensions
//                 obj.GetDimensionArray(),
//                 // Strides (in bytes) for each index
//                 obj.GetByteStrides(),
//                 //readonly flag is false
//                 false
//             );
//         } );
// }


}//end of hops namespace

#endif /* end of include guard: MHO_PyNDArrayWrapper */
