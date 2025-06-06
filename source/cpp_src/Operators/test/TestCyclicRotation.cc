#include "MHO_CyclicRotator.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"

#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>

using namespace hops;

#define NDIM 2
typedef MHO_NDArrayWrapper< std::complex< double >, NDIM > array_type;
typedef MHO_NDArrayWrapper< std::complex< double >, 1 > array_type1d;

int main(int /*argc*/, char** /*argv*/)
{

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    const size_t ndim = NDIM;
    const size_t dval = 8;
    size_t dim_size[ndim];
    for(std::size_t i = 0; i < NDIM; i++)
    {
        dim_size[i] = i + dval;
    };
    array_type* input1 = new array_type(dim_size);
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
            (*input1)(i, j) = std::complex< double >(i, j);
            std::cout << (*input1)(i, j) << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    MHO_CyclicRotator< array_type > crot;
    crot.SetOffset(0, 2);
    crot.SetOffset(1, -2);
    crot.SetArgs(input1, output);
    bool init = crot.Initialize();
    bool exe = crot.Execute();

    size_t odim_size[NDIM];
    output->GetDimensions(odim_size);

    for(size_t i = 0; i < NDIM; i++)
    {
        std::cout << "out dim @ " << i << " = " << odim_size[i] << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    for(size_t i = 0; i < odim_size[0]; i++)
    {
        for(size_t j = 0; j < odim_size[1]; j++)
        {
            std::cout << (*output)(i, j) << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    //now undo the previous rotation in-place on the output array
    crot.SetOffset(0, -2);
    crot.SetOffset(1, 2);
    crot.SetArgs(output);
    bool init2 = crot.Initialize();
    bool exe2 = crot.Execute();

    for(size_t i = 0; i < odim_size[0]; i++)
    {
        for(size_t j = 0; j < odim_size[1]; j++)
        {
            std::cout << (*output)(i, j) << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;

    size_t dim_size1d[1];
    dim_size1d[0] = 8;
    array_type1d* input1d = new array_type1d(dim_size1d[0]);
    array_type1d* output1d = new array_type1d();

    for(size_t j = 0; j < dim_size1d[0]; j++)
    {
        (*input1d)[j] = j;
        std::cout << (*input1d)[j] << ", ";
    }

    std::cout << std::endl;

    MHO_CyclicRotator< array_type1d > crot1d;
    crot1d.SetOffset(0, 2);
    crot1d.SetArgs(input1d, output1d);
    bool init3 = crot1d.Initialize();
    bool exe3 = crot1d.Execute();

    for(size_t j = 0; j < dim_size1d[0]; j++)
    {
        std::cout << (*output1d)[j] << ", ";
    }
    std::cout << std::endl;

    return 0;
}
