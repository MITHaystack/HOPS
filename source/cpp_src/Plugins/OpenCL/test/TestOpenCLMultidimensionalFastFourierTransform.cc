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



unsigned int fNLocal;
unsigned int fPreferredWorkgroupMultiple;
unsigned int fMaxNWorkItems;
cl::Kernel* fFFTKernel = nullptr;
void ConstructOpenCLKernels()
{
    std::cout<<"building opencl kernels"<<std::endl;
    //Get name of kernel source file
    std::stringstream clFile;
    clFile << MHO_OpenCLInterface::GetInstance()->GetKernelPath() << "/MHO_MultidimensionalFastFourierTransform_kernel.cl";

    //set the build options
    std::stringstream options;
    //create the build flags
    std::stringstream ss;
    ss << " -D FFT_NDIM=" << NDIM;
    ss << " -I " << MHO_OpenCLInterface::GetInstance()->GetKernelPath();

    options << ss.str();

    MHO_OpenCLKernelBuilder k_builder;
    fFFTKernel = k_builder.BuildKernel(clFile.str(), std::string("MultidimensionalFastFourierTransform_Radix2Stage"), options.str());

    //get n-local
    fNLocal = fFFTKernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(MHO_OpenCLInterface::GetInstance()->GetDevice());
    fPreferredWorkgroupMultiple = fFFTKernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>( MHO_OpenCLInterface::GetInstance()->GetDevice());
    if (fPreferredWorkgroupMultiple < fNLocal){fNLocal = fPreferredWorkgroupMultiple;}

    //determine the largest global worksize
    fMaxNWorkItems = 0;
    // for (unsigned int D = 0; D < XArgType::rank::value; D++) {
    //     //compute number of 1d fft's needed (n-global)
    //     unsigned int n_global = fDimensionSize[0];
    //     unsigned int n_local_1d_transforms = 1;
    //     for (unsigned int i = 0; i < XArgType::rank::value-1; i++) {
    //         if (i != D) {
    //             n_global *= fSpatialDim[i];
    //             n_local_1d_transforms *= fSpatialDim[i];
    //         };
    //     };
    // 
    //     //pad out n-global to be a multiple of the n-local
    //     unsigned int nDummy = fNLocal - (n_global % fNLocal);
    //     if (nDummy == fNLocal) {
    //         nDummy = 0;
    //     };
    //     n_global += nDummy;
    // 
    //     if (fMaxNWorkItems < n_global) {
    //         fMaxNWorkItems = n_global;
    //     };
    // }


    std::cout<<"fNLocal = "<<fNLocal<<std::endl;
    std::cout<<"fPreferredWorkgroupMultiple = "<<fPreferredWorkgroupMultiple<<std::endl;
    std::cout<<"fMaxNWorkItems = "<<fMaxNWorkItems<<std::endl;
    
}

cl::Buffer* fDimensionBufferCL = nullptr;
cl::Buffer* fTwiddleBufferCL = nullptr;
cl::Buffer* fConjugateTwiddleBufferCL = nullptr;
cl::Buffer* fScaleBufferCL = nullptr;
cl::Buffer* fCirculantBufferCL = nullptr;
cl::Buffer* fDataBufferCL = nullptr;
cl::Buffer* fPermuationArrayCL = nullptr;

void AllocateDeviceWorkspace(unsigned int fMaxBufferSize)
{
    std::cout<<"building CL buffers"<<std::endl;
    fDimensionBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_ONLY, NDIM * sizeof(unsigned int));
    fTwiddleBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
    fConjugateTwiddleBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
    fScaleBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
    fCirculantBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
    fPermuationArrayCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_ONLY,fMaxBufferSize * sizeof(unsigned int));
}




int main(int /*argc*/, char** /*argv*/)
{
    MHO_OpenCLInterface::GetInstance();

    size_t dim[NDIM];
    dim[0] = 256; //x
    dim[1] = 256; //y
    dim[2] = 64; //z

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

    AllocateDeviceWorkspace(256);
    ConstructOpenCLKernels();


    delete fDimensionBufferCL;
    delete fTwiddleBufferCL;
    delete fConjugateTwiddleBufferCL;
    delete fScaleBufferCL;
    delete fCirculantBufferCL;
    delete fDataBufferCL;
    delete fPermuationArrayCL;

    delete fFFTKernel;
    // buffer_ext->WriteDataBuffer();
    // buffer_ext->WriteDimensionBuffer();

    // MHO_OpenCLScalarMultiply< std::complex<double>, test_table_type> scalarMult;
    // std::complex<double> factor = std::complex<double>(2.0,0.0);
    // scalarMult.SetFactor(factor);
    // scalarMult.SetReadTrue();
    // scalarMult.SetWriteTrue();
    // scalarMult.SetArgs(test);
    // scalarMult.Initialize();
    // scalarMult.Execute();
    // 
    // 
    // //now lets take a look at the data:
    // std::cout<<"**********************"<<std::endl;
    // std::cout<<"using device to scale by factor of: "<<factor<<std::endl;
    // std::cout<<"**********************"<<std::endl;
    // std::cout<<"test(0,0,0) = "<<(*test)(0,0,0)<<std::endl;
    // std::cout<<"test(1,1,1) = "<<(*test)(1,1,1)<<std::endl;
    // std::cout<<"test(2,2,2) = "<<(*test)(2,2,2)<<std::endl;


    return 0;
}
