#include "MHO_Message.hh"
#include "MHO_EndZeroPadder.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_Axis.hh"
#include "MHO_TableContainer.hh"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <complex>


using namespace hops;

#define NDIM 2
typedef MHO_NDArrayWrapper<std::complex<double>, NDIM> array_type;

using axis1_type = MHO_Axis<int>;
using axis2_type = MHO_Axis<double>;
using axis_pack_type = MHO_AxisPack< axis1_type, axis2_type>;
using table_type = MHO_TableContainer< double, axis_pack_type >;

int main(int /*argc*/, char** /*argv*/)
{

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    const size_t ndim = NDIM;
    const size_t dval = 3;
    size_t dim_size[ndim];
    for(std::size_t i=0;i<NDIM;i++){dim_size[i] = dval;};
    array_type* input1 = new array_type(dim_size);
    array_type* output = new array_type();

    size_t idim_size[NDIM];
    input1->GetDimensions(idim_size);

    for(size_t i=0;i<NDIM;i++)
    {
        std::cout<<" in dim @ "<<i<< " = "<<idim_size[i]<<std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < idim_size[0]; i++) {
        for (size_t j = 0; j < idim_size[1]; j++) {
                (*input1)(i,j) = std::complex<double>(i,j);
                std::cout << (*input1)(i,j) << ", ";
        }
            std::cout << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    std::size_t M = 5;
    MHO_EndZeroPadder<array_type> padder;
    padder.SetPaddingFactor(M);
    padder.SetEndPadded();
    padder.SetArgs(input1, output);
    bool init = padder.Initialize();
    bool exe = padder.Execute();

    size_t odim_size[NDIM];
    output->GetDimensions(odim_size);

    for(size_t i=0;i<NDIM;i++)
    {
        std::cout<<"out dim @ "<<i<<" = "<<odim_size[i]<<std::endl;
    }

    for (size_t i = 0; i < odim_size[0]; i++) {
        for (size_t j = 0; j < odim_size[1]; j++) {
                std::cout << (*output)(i,j) << ", ";
        }
            std::cout << std::endl;
    }

    return 0;
}
