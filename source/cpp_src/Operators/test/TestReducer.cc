#include "MHO_Message.hh"
#include "MHO_Reducer.hh"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <complex>


using namespace hops;

#define NDIM 3

using array_type = MHO_NDArrayWrapper<std::complex<double>, NDIM>;

int main(int /*argc*/, char** /*argv*/)
{
    const size_t ndim = NDIM;
    const size_t dval = 3;
    size_t dim_size[ndim];
    for(std::size_t i=0;i<NDIM;i++){dim_size[i] = i+dval;};
    MHO_NDArrayWrapper<std::complex<double>, ndim>* input = new MHO_NDArrayWrapper<std::complex<double>, ndim>(dim_size);
    MHO_NDArrayWrapper<std::complex<double>, ndim>* output = new MHO_NDArrayWrapper<std::complex<double>, ndim>();
    MHO_NDArrayWrapper<std::complex<double>, ndim>* output2 = new MHO_NDArrayWrapper<std::complex<double>, ndim>();

    input->SetArray( std::complex<double>(2.0,0.0) );

    size_t idim_size[NDIM];
    input->GetDimensions(idim_size);

    for(size_t i=0;i<NDIM;i++)
    {
        std::cout<<" in dim @ "<<i<< " = "<<idim_size[i]<<std::endl;
    }


    for (size_t i = 0; i < idim_size[0]; i++) {
        for (size_t j = 0; j < idim_size[1]; j++) {
            for (size_t k = 0; k < idim_size[2]; k++) {
                std::cout << (*input)(i,j,k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout <<" ***** "<< std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    MHO_Reducer< array_type, MHO_CompoundSum>* reducer = new MHO_Reducer<array_type, MHO_CompoundSum>();
    reducer->SetArgs(input, output);
    //reducer->ReduceAxis(0);
    reducer->ReduceAxis(NDIM-2);
    //reducer->ReduceAxis(NDIM-1);
    bool init = reducer->Initialize();
    bool exe = reducer->Execute();
    size_t odim_size[NDIM];
    output->GetDimensions(odim_size);


    for(size_t i=0;i<NDIM;i++)
    {
        std::cout<<"out dim @ "<<i<<" = "<<odim_size[i]<<std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;


    for (size_t i = 0; i < odim_size[0]; i++) {
        for (size_t j = 0; j < odim_size[1]; j++) {
            for (size_t k = 0; k < odim_size[2]; k++) {
                std::cout << (*output)(i,j,k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout <<" ***** "<< std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    MHO_Reducer<array_type, MHO_CompoundMultiply>* reducer2 = new MHO_Reducer<array_type, MHO_CompoundMultiply>();
    reducer2->SetArgs(output, output2);
    //reducer->ReduceAxis(0);
    //reducer->ReduceAxis(NDIM-2);
    reducer2->ReduceAxis(NDIM-1);
    bool init2 = reducer2->Initialize();
    bool exe2 = reducer2->Execute();
    output2->GetDimensions(odim_size);


    for(size_t i=0;i<NDIM;i++)
    {
        std::cout<<"out dim @ "<<i<<" = "<<odim_size[i]<<std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < odim_size[0]; i++) {
        for (size_t j = 0; j < odim_size[1]; j++) {
            for (size_t k = 0; k < odim_size[2]; k++) {
                std::cout << (*output2)(i,j,k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout <<" ***** "<< std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;



    return 0;
}
