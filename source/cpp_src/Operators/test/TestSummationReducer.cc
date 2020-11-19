#include "MHOMessage.hh"
#include "MHOSummationReducer.hh"

#include <cmath>
#include <iomanip>
#include <iostream>


using namespace hops;

#define NDIM 3

int main(int /*argc*/, char** /*argv*/)
{
    const size_t ndim = NDIM;
    const size_t dval = 3;
    size_t dim_size[ndim];
    for(std::size_t i=0;i<NDIM;i++){dim_size[i] = i+dval;};
    MHOArrayWrapper<double, ndim>* input = new MHOArrayWrapper<double, ndim>(dim_size);
    MHOArrayWrapper<double, ndim>* output = new MHOArrayWrapper<double, ndim>();

    input->SetArray(1.0);

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

    MHOSummationReducer<double,NDIM>* reducer = new MHOSummationReducer<double,NDIM>();
    reducer->SetInput(input);
    reducer->SetOutput(output);
    //reducer->ReduceAxis(0);
    reducer->ReduceAxis(NDIM-2);
    //reducer->ReduceAxis(NDIM-1);
    bool init = reducer->Initialize();
    bool exe = reducer->ExecuteOperation();

    std::cout<<"exe = "<<exe<<std::endl;

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

    return 0;
}
