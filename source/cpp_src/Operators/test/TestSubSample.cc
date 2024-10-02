#include "MHO_Axis.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_SubSample.hh"
#include "MHO_TableContainer.hh"

#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>

using namespace hops;

#define NDIM 2
typedef MHO_NDArrayWrapper< std::complex< double >, NDIM > array_type;

using axis1_type = MHO_Axis< int >;
using axis2_type = MHO_Axis< double >;
using axis_pack_type = MHO_AxisPack< axis1_type, axis2_type >;
using table_type = MHO_TableContainer< double, axis_pack_type >;

int main(int /*argc*/, char** /*argv*/)
{

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    const size_t ndim = NDIM;
    const size_t dval = 8;
    size_t dim_size[ndim];
    for(std::size_t i = 0; i < NDIM; i++)
    {
        dim_size[i] = dval;
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

    MHO_SubSample< array_type > sub;
    sub.SetDimensionAndStride(1, 2);
    sub.SetArgs(input1, output);
    // sub.SetInput(input1);
    // sub.SetOutput(output);
    bool init = sub.Initialize();
    bool exe = sub.Execute();

    size_t odim_size[NDIM];
    output->GetDimensions(odim_size);

    for(size_t i = 0; i < NDIM; i++)
    {
        std::cout << "out dim @ " << i << " = " << odim_size[i] << std::endl;
    }

    for(size_t i = 0; i < odim_size[0]; i++)
    {
        for(size_t j = 0; j < odim_size[1]; j++)
        {
            std::cout << (*output)(i, j) << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    table_type* table_in = new table_type();
    table_type* table_out = new table_type();
    odim_size[0] = 4;
    odim_size[1] = 8;
    table_in->Resize(odim_size);
    auto ax1 = &(std::get< 1 >(*table_in));
    std::cout << "ax1 size = " << ax1->GetSize() << std::endl;
    for(std::size_t i = 0; i < ax1->GetSize(); i++)
    {
        (*ax1)(i) = i;
        std::cout << "ax1 @" << i << " = " << (*ax1)(i) << std::endl;
    }
    MHO_SubSample< table_type > sub2;
    sub2.SetDimensionAndStride(1, 2);
    sub2.SetArgs(table_in, table_out);
    sub2.Initialize();
    sub2.Execute();

    auto ax2 = &(std::get< 1 >(*table_out));
    for(std::size_t i = 0; i < ax2->GetSize(); i++)
    {
        std::cout << "ax2 @" << i << " = " << (*ax2)(i) << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    //    table_in->Resize(odim_size);
    // auto ax1 = &(std::get<1>(*table_in));
    // std::cout<<"ax1 size = "<<ax1->GetSize()<<std::endl;
    // for(std::size_t i=0; i<ax1->GetSize(); i++)
    // {
    //     (*ax1)(i) = i;
    //     std::cout<<"ax1 @"<<i<<" = "<<(*ax1)(i)<<std::endl;
    // }
    sub2.SetDimensionAndStride(1, 2);
    sub2.SetArgs(table_in); //test the in place operation
    sub2.Initialize();
    sub2.Execute();

    auto ax3 = &(std::get< 1 >(*table_in));
    for(std::size_t i = 0; i < ax3->GetSize(); i++)
    {
        std::cout << "ax3 @" << i << " = " << (*ax3)(i) << std::endl;
    }

    auto new_dims = table_in->GetDimensionArray();
    std::cout << "new dimensions = " << new_dims[0] << ", " << new_dims[1] << std::endl;

    return 0;
}
