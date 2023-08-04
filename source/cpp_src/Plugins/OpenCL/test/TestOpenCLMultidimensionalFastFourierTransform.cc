#include "MHO_Message.hh"

#include <cmath>
#include <iostream>

#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"
#include "MHO_OpenCLNDArrayBuffer.hh"
#include "MHO_TableContainer.hh"

#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"


using namespace hops;

#define NDIM 3
#define XDIM 0
#define YDIM 1
#define ZDIM 2
typedef MHO_AxisPack< MHO_Axis<double>, MHO_Axis<double>, MHO_Axis< std::string > > axis_pack_test;
typedef MHO_TableContainer< std::complex<double>, axis_pack_test > test_table_type;

typedef MHO_NDArrayWrapper< std::complex<double>, 1 > twiddle_type; 
typedef MHO_NDArrayWrapper< unsigned int, 1 > permutation_array_type; 

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

    std::cout<<"fNLocal = "<<fNLocal<<std::endl;
    std::cout<<"fPreferredWorkgroupMultiple = "<<fPreferredWorkgroupMultiple<<std::endl;
    std::cout<<"fMaxNWorkItems = "<<fMaxNWorkItems<<std::endl;
    
}



int main(int /*argc*/, char** /*argv*/)
{
    MHO_OpenCLInterface::GetInstance();

    size_t dim[NDIM];
    dim[0] = 4; //x
    dim[1] = 4; //y
    dim[2] = 4; //z

    test_table_type* test = new test_table_type(dim);
    test_table_type* test2 = new test_table_type(dim);

    std::size_t total_size = test->GetSize();
    //fill up the array with data 
    for(std::size_t i=0; i<total_size; i++)
    {
        (*test)[i] = std::complex<double>(i % 13, i % 17); 
        (*test2)[i] = std::complex<double>(i % 13, i % 17); //for read back testing
    }

    std::cout<<"test(0,0,0) = "<<(*test)(0,0,0)<<std::endl;
    std::cout<<"test(1,1,1) = "<<(*test)(1,1,1)<<std::endl;
    std::cout<<"test(2,2,2) = "<<(*test)(2,2,2)<<std::endl;

    //host/device workspace for FFT plan info 
    MHO_FastFourierTransformWorkspace<double> fW[NDIM];
    twiddle_type fTwiddle[NDIM];
    permutation_array_type fPerm[NDIM];
    for(unsigned int i=0; i<NDIM; i++)
    {
        fW[i].Resize(dim[i]);
        fTwiddle[i].Resize(fW[i].fN);
        fPerm[i].Resize(fW[i].fN);
        for(std::size_t j=0; j<fW[i].fN; j++)
        {
            fTwiddle[i][j] = fW[i].fTwiddle[j];
            fPerm[i][j] = fW[i].fPermutation[j];
        }
    }


    ConstructOpenCLKernels();

    //create the opencl buffer extensions 
    ///data and dims first
    auto buffer_ext = test->MakeExtension< MHO_OpenCLNDArrayBuffer< test_table_type > >();
    buffer_ext->ConstructDimensionBuffer();
    buffer_ext->ConstructDataBuffer();
    //write data to the GPU
    // buffer_ext->WriteDataBuffer();
    buffer_ext->WriteDimensionBuffer();

    //then create the buffers for the FFT plan info 
    for(unsigned int i=0; i<NDIM; i++)
    {
        auto twid_ext = fTwiddle[i].MakeExtension< MHO_OpenCLNDArrayBuffer< twiddle_type > >();
        twid_ext->ConstructDataBuffer();
        twid_ext->WriteDataBuffer();
        auto perm_ext = fPerm[i].MakeExtension< MHO_OpenCLNDArrayBuffer< permutation_array_type > >();
        perm_ext->ConstructDataBuffer();
        perm_ext->WriteDataBuffer();
    }


    //determine the largest global worksize
    fMaxNWorkItems = 0;
    unsigned int n_global;
    //now run the FFT stages (update the twiddle/perm at each stage)
    for(unsigned int D=0; D<NDIM; D++)
    {
        //compute number of 1d fft's needed (n-global)
        unsigned int n_global = 1;
        for(unsigned int i = 0; i < NDIM; i++){ if (i != D){n_global *= dim[i];} }
        //pad out n-global to be a multiple of the n-local
        unsigned int nDummy = fNLocal - (n_global % fNLocal);
        if(nDummy == fNLocal){ nDummy = 0; }
        n_global += nDummy;
        if (fMaxNWorkItems < n_global){ fMaxNWorkItems = n_global; }

        std::cout<<"dim = "<<dim[D]<<" n local = "<<fNLocal<<" n_global "<<n_global<<std::endl;

        cl::NDRange global(n_global);
        cl::NDRange local(fNLocal);

        // unsigned int D, //d = 0, 1, ...FFT_NDIM-1 specifies the dimension/axis selected to be transformed
        // __global const unsigned int* array_dimensions, //sizes of the array in each dimension
        // __global const CL_TYPE2* twiddle, //fft twiddle factors
        // __global const unsigned int* permutation_array, //bit reversal permutation indices
        // __global CL_TYPE2* data // the data to be transformed (in-place)

        //set the arguments which are updated at each stage
        fFFTKernel->setArg(0, D);
        std::cout<<"flag0"<<std::endl;
        std::cout<<"ptr = "<<buffer_ext->GetDimensionBuffer()<<std::endl;

        CL_ERROR_TRY
        fFFTKernel->setArg(1, *( buffer_ext->GetDimensionBuffer() ) );
        CL_ERROR_CATCH
        std::cout<<"flag1"<<std::endl;

        CL_ERROR_TRY
        fFFTKernel->setArg(2, *( fTwiddle[D].AsExtension< MHO_OpenCLNDArrayBuffer< twiddle_type > >()->GetDataBuffer() ) );
        CL_ERROR_CATCH
        std::cout<<"flag2"<<std::endl;

        CL_ERROR_TRY
        fFFTKernel->setArg(3, *( fPerm[D].AsExtension< MHO_OpenCLNDArrayBuffer< permutation_array_type > >()->GetDataBuffer() ) );
        CL_ERROR_CATCH
        std::cout<<"flag3"<<std::endl;

        CL_ERROR_TRY
        fFFTKernel->setArg(4, *( buffer_ext->GetDataBuffer() ) );
        CL_ERROR_CATCH
        std::cout<<"flag4"<<std::endl;

        //now enqueue the kernel
        MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueNDRangeKernel(*fFFTKernel,
                                                                         cl::NullRange,
                                                                         global,
                                                                         local);

        //force it to finish
        MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
    }

    //get the results
    buffer_ext->ReadDataBuffer();

    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();

    std::cout<<"test(0,0,0) = "<<(*test)(0,0,0)<<std::endl;
    std::cout<<"test(1,1,1) = "<<(*test)(1,1,1)<<std::endl;
    std::cout<<"test(2,2,2) = "<<(*test)(2,2,2)<<std::endl;

    // //now do an IFFT on the CPU to check we get the same thing back.
    // auto fft_engine = new MHO_MultidimensionalFastFourierTransform< test_table_type >();
    // //no do IFFT pass on all axes
    // fft_engine->SetBackward();
    // fft_engine->SetArgs(test);
    // fft_engine->SelectAllAxes();
    // fft_engine->Initialize();
    // fft_engine->Execute();
    // 
    // std::cout<<"test(0,0,0) = "<<(*test)(0,0,0)<<std::endl;
    // std::cout<<"test(1,1,1) = "<<(*test)(1,1,1)<<std::endl;
    // std::cout<<"test(2,2,2) = "<<(*test)(2,2,2)<<std::endl;

    std::cout<< *test << std::endl;

    //clean up
    delete fFFTKernel;
    //delete fft_engine;

    return 0;
}
