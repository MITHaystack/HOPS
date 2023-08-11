#include "MHO_Message.hh"
#include "MHO_Timer.hh"

#include <cmath>
#include <iostream>

#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"
#include "MHO_OpenCLNDArrayBuffer.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_FastFourierTransformWorkspace.hh"

using namespace hops;

#define USE_FFTW

#define FP_Type double
//#define FP_Type float  (can only be use if CL_TYPE is defined as float)

#define NDIM 3
#define MAX_NBITS 32
#define MAX_CONCURRENT_WORKGROUPS 4
typedef MHO_AxisPack< MHO_Axis<FP_Type>, MHO_Axis<FP_Type>, MHO_Axis<FP_Type> > axis_pack_test;
typedef MHO_TableContainer< std::complex<FP_Type>, axis_pack_test > test_table_type;

typedef MHO_NDArrayWrapper< std::complex<FP_Type>, 1 > circulant_type;

#ifdef USE_FFTW
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#define CPU_FFT_TYPE MHO_MultidimensionalFastFourierTransformFFTW<test_table_type>
#else 
#define CPU_FFT_TYPE MHO_MultidimensionalFastFourierTransform<test_table_type>
#endif


unsigned int fNLocal;
unsigned int fPreferredWorkgroupMultiple;
unsigned int fMaxNWorkItems;
cl::Kernel* fFFTKernel = nullptr;
void ConstructOpenCLKernels()
{
    std::cout<<"building opencl kernels"<<std::endl;
    //Get name of kernel source file
    std::stringstream clFile;
    clFile << MHO_OpenCLInterface::GetInstance()->GetKernelPath() << "/MHO_NDFFTBluestein_kernel.cl";

    //set the build options
    std::stringstream options;
    //create the build flags
    std::stringstream ss;
    ss << " -D FFT_NDIM=" << NDIM;
    ss << " -I " << MHO_OpenCLInterface::GetInstance()->GetKernelPath();

    options << ss.str();
    MHO_OpenCLKernelBuilder k_builder;
    fFFTKernel = k_builder.BuildKernel(clFile.str(), std::string("NDFFTBluesteinStage"), options.str());

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
    MHO_Timer timer;
    size_t dim[NDIM];

    dim[0] = 56; //x
    dim[1] = 30; //y
    dim[2] = 160; //z

    test_table_type* test = new test_table_type(dim);
    test_table_type* test2 = new test_table_type(dim);

    std::size_t total_size = test->GetSize();
    //fill up the array with data 
    for(std::size_t i=0; i<total_size; i++)
    {
        (*test)[i] = std::complex<FP_Type>(i % 5, i % 17); 
        (*test2)[i] = std::complex<FP_Type>(i % 5, i % 17); 
    }

    ConstructOpenCLKernels();

    std::cout<<"creating data buffer"<<std::endl;
    //create the opencl buffer extensions 
    ///data and dims first
    auto buffer_ext = test->MakeExtension< MHO_OpenCLNDArrayBuffer< test_table_type > >();
    buffer_ext->ConstructDimensionBuffer();
    buffer_ext->ConstructDataBuffer();
    //write data to the GPU
    buffer_ext->WriteDataBuffer();
    buffer_ext->WriteDimensionBuffer();

    //we need to construct FFT workspace plans to build the scale/circulant vectors 
    MHO_FastFourierTransformWorkspace<FP_Type> fft_work[NDIM];
    unsigned int max_dim_size = 0;
    std::vector< circulant_type > scales;
    std::vector< circulant_type > circs;
    std::vector< cl::Buffer* > scale_buffers;
    std::vector< cl::Buffer* > circ_buffers;
    for(unsigned int i=0; i<NDIM; i++)
    {
        fft_work[i].Resize(dim[i]);
        if(dim[i] > max_dim_size){max_dim_size = dim[i];}

        circulant_type s;
        circulant_type c;
        s.Resize(fft_work[i].fM);
        c.Resize(fft_work[i].fM);

        for(unsigned int j=0; j<fft_work[i].fM; j++)
        {
            s[j] = fft_work[i].fScale[j];
            c[j] = fft_work[i].fCirculant[j];
        }

        auto s_ext = s.MakeExtension< MHO_OpenCLNDArrayBuffer< circulant_type > >();
        s_ext->ConstructDataBuffer();
        s_ext->WriteDataBuffer();

        auto c_ext = c.MakeExtension< MHO_OpenCLNDArrayBuffer< circulant_type > >();
        c_ext->ConstructDataBuffer();
        c_ext->WriteDataBuffer();

        scales.push_back(s);
        circs.push_back(c);

        scale_buffers.push_back(s_ext->GetDataBuffer() );
        circ_buffers.push_back(c_ext->GetDataBuffer() );
    }

    // unsigned int D, //d = 0, 1, ...FFT_NDIM-1 specifies the dimension/axis selected to be transformed
    // __global const unsigned int* dim_arr, //sizes of the array in each dimension
    // __local CL_TYPE2* twiddle_scratch, //scratch space for the twiddle factor basis
    // __global CL_TYPE2* data, // the data to be transformed
    // __global CL_TYPE2* scale,
    // __global CL_TYPE2* circulant,
    // __global CL_TYPE2* workspace)

    std::cout<<"total size = "<<total_size<<std::endl;

    fFFTKernel->setArg(1, *( buffer_ext->GetDimensionBuffer() ) ); //array dimensions
    fFFTKernel->setArg(2, MAX_NBITS*fNLocal*sizeof(cl_double2), NULL);
    fFFTKernel->setArg(3, *( buffer_ext->GetDataBuffer() ) ); 

    //create a workspace buffer (arg 6)
    unsigned int n_bytes = (static_cast< unsigned int >( MAX_CONCURRENT_WORKGROUPS*fNLocal*max_dim_size) )*sizeof( MHO_OpenCLTypeMap< std::complex<double>  >::mapped_type );  
    cl::Buffer* fWorkspaceBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_WRITE, n_bytes);
    fFFTKernel->setArg(6, *fWorkspaceBufferCL );

    //determine the largest global worksize
    fMaxNWorkItems = 0;
    unsigned int n_global;
    //now run the FFT stages (update the twiddle/perm at each stage)

    timer.MeasureWallclockTime();
    timer.Start();
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
        //std::cout<<"D = "<<D<<" dim = "<<dim[D]<<" n local = "<<fNLocal<<" n_global "<<n_global<<" ndummy = "<<nDummy<<std::endl;

        


        cl::NDRange global(n_global);
        cl::NDRange local(fNLocal);

        //set the arguments which are updated at each stage
        fFFTKernel->setArg(0, D); //selected dimension

        std::cout<<"pter = "<<scale_buffers[D]<<std::endl;
        CL_ERROR_TRY
        fFFTKernel->setArg(4, *(scale_buffers[D]) ); 
        CL_ERROR_CATCH

        CL_ERROR_TRY
        fFFTKernel->setArg(5, *(circ_buffers[D]) ); 
        CL_ERROR_CATCH

        //now enqueue the kernel
        MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueNDRangeKernel(*fFFTKernel,
                                                                         cl::NullRange,
                                                                         global,
                                                                         local);
    }

    //get the results (move this out of loop)
    buffer_ext->ReadDataBuffer();
    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
    
    timer.Stop();
    FP_Type gpu_runtime = timer.GetDurationAsDouble();
    std::cout<<"GPU time = "<<gpu_runtime<<std::endl;

    //now do an FFT on the CPU to check we get the same thing
    auto fft_engine = new CPU_FFT_TYPE();
    //no do IFFT pass on all axes
    fft_engine->SetForward();
    fft_engine->SetArgs(test2);
    fft_engine->DisableAxisLabelTransformation();
    fft_engine->SelectAllAxes();
    fft_engine->Initialize();

    timer.MeasureWallclockTime();
    timer.Start();
    fft_engine->Execute();
    timer.Stop();
    FP_Type cpu_runtime = timer.GetDurationAsDouble();

    std::cout<<"CPU time = "<<cpu_runtime<<std::endl;
    std::cout<<"speed up factor (should be >1) = "<<cpu_runtime/gpu_runtime<<std::endl;

    //compute the difference between the results
    FP_Type delta = 0;
    for(std::size_t i=0; i<test->GetSize(); i++)
    {
        delta += std::abs( (*test)[i] - (*test2)[i] );
    }
    std::cout<<"average delta = "<<delta/(FP_Type)total_size<<std::endl;


    //clean up
    delete fFFTKernel;
    delete fft_engine;

    return 0;
}
