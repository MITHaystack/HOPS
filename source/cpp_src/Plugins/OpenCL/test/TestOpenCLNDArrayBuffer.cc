#include "MHO_Message.hh"

#include <cmath>
#include <iostream>

#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"
#include "MHO_OpenCLNDArrayBuffer.hh"
#include "MHO_TableContainer.hh"

#include "MHO_OpenCLScalarMultiply.hh"


using namespace hops;

#define NDIM 3
#define XDIM 0
#define YDIM 1
#define ZDIM 2
typedef MHO_AxisPack< MHO_Axis<double>, MHO_Axis<double>, MHO_Axis< std::string > > axis_pack_test;
typedef MHO_TableContainer< std::complex<double>, axis_pack_test > test_table_type;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_OpenCLInterface::GetInstance();

    size_t dim[NDIM];
    dim[0] = 256; //x
    dim[1] = 256; //y
    dim[2] = 3; // r,g,b

    test_table_type* test = new test_table_type(dim);

    //set values
    for(size_t i=0; i<dim[0]; i++)
    {
        for(size_t j=0; j<dim[1]; j++)
        {
            for(size_t k=0; k<dim[2]; k++)
            {
                double value = 10*i + j;
                (*test)(i,j,k) = std::complex<double>(value, k+1);
            }
        }
    }

    std::cout<<"test(0,0,0) = "<<(*test)(0,0,0)<<std::endl;
    std::cout<<"test(1,1,1) = "<<(*test)(1,1,1)<<std::endl;
    std::cout<<"test(2,2,2) = "<<(*test)(2,2,2)<<std::endl;

    //create the buffer extension
    auto buffer_ext = test->MakeExtension< MHO_OpenCLNDArrayBuffer< test_table_type > >();

    buffer_ext->ConstructDimensionBuffer();
    buffer_ext->ConstructDataBuffer();
    // buffer_ext->WriteDataBuffer();
    // buffer_ext->WriteDimensionBuffer();

    MHO_OpenCLScalarMultiply< std::complex<double>, test_table_type> scalarMult;
    std::complex<double> factor = std::complex<double>(3.0,1.0);
    scalarMult.SetFactor(factor);
    scalarMult.SetReadTrue();
    scalarMult.SetWriteTrue();
    scalarMult.SetInput(test);
    scalarMult.Initialize();
    scalarMult.ExecuteOperation();

    //now lets take a look at the data:
    std::cout<<"**********************"<<std::endl;
    std::cout<<"using device to scale by factor of: "<<factor<<std::endl;
    std::cout<<"**********************"<<std::endl;
    std::cout<<"test(0,0,0) = "<<(*test)(0,0,0)<<std::endl;
    std::cout<<"test(1,1,1) = "<<(*test)(1,1,1)<<std::endl;
    std::cout<<"test(2,2,2) = "<<(*test)(2,2,2)<<std::endl;

    return 0;
}
