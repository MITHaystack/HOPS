#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"

#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>

using namespace hops;

#define NDIM 3
typedef MHO_NDArrayWrapper< std::complex< double >, NDIM > array_type;

int main(int /*argc*/, char** /*argv*/)
{

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    const size_t ndim = NDIM;
    const size_t dval = 3;
    size_t dim_size[ndim];
    for(std::size_t i = 0; i < NDIM; i++)
    {
        dim_size[i] = i + dval;
    };
    array_type* input1 = new array_type(dim_size);
    array_type* input2 = new array_type(dim_size);
    array_type* output = new array_type();

    size_t idim_size[NDIM];
    input1->GetDimensions(idim_size);

    for(size_t i = 0; i < NDIM; i++)
    {
        std::cout << " in dim @ " << i << " = " << idim_size[i] << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    for(size_t i = 0; i < idim_size[0]; i++)
    {
        for(size_t j = 0; j < idim_size[1]; j++)
        {
            for(size_t k = 0; k < idim_size[2]; k++)
            {
                (*input1)(i, j, k) = i;
                std::cout << (*input1)(i, j, k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout << " ***** " << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    for(size_t i = 0; i < idim_size[0]; i++)
    {
        for(size_t j = 0; j < idim_size[1]; j++)
        {
            for(size_t k = 0; k < idim_size[2]; k++)
            {
                (*input2)(i, j, k) = j;
                std::cout << (*input2)(i, j, k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout << " ***** " << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    //in-place pointwise multiply
    *input1 *= *input2;

    //
    size_t odim_size[NDIM];
    input1->GetDimensions(odim_size);

    for(size_t i = 0; i < NDIM; i++)
    {
        std::cout << "out dim @ " << i << " = " << odim_size[i] << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    for(size_t i = 0; i < odim_size[0]; i++)
    {
        for(size_t j = 0; j < odim_size[1]; j++)
        {
            for(size_t k = 0; k < odim_size[2]; k++)
            {
                std::cout << (*input1)(i, j, k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout << " ***** " << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    return 0;
}
