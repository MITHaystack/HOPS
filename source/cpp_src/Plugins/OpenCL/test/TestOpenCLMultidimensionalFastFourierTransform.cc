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

#define NDIM 2

typedef MHO_AxisPack< MHO_Axis<double>, MHO_Axis<double> > axis_pack_test;
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
    dim[0] = 8; //x
    dim[1] = 8; //y
    // dim[2] = 4; //z

    test_table_type* test = new test_table_type(dim);
    test_table_type* test2 = new test_table_type(dim);

    std::size_t total_size = test->GetSize();
    //fill up the array with data 
    for(std::size_t i=0; i<total_size; i++)
    {
        (*test)[i] = std::complex<double>(  (i+1)%2, i % 2); 
        (*test2)[i] = std::complex<double>( (i+1)%2, i % 2);
        //(*test2)[i] = std::complex<double>(i % 13, i % 17); //for read back testing
    }
    // (*test)(0,0) = std::complex<double>(1.0, 0.);
    // (*test2)(0,0) = std::complex<double>(1.0, 0.);
    // (*test)(1,0) = std::complex<double>(1.0, 0.);
    // (*test2)(1,0) = std::complex<double>(1.0, 0.);
    // (*test)(2,0) = std::complex<double>(1.0, 0.);
    // (*test2)(2,0) = std::complex<double>(1.0, 0.);
    // (*test)(3,0) = std::complex<double>(1.0, 0.);
    // (*test2)(3,0) = std::complex<double>(1.0, 0.);


    std::cout<<"original array = "<<std::endl;
    std::cout<<(*test)<<std::endl;
    
    std::cout<<"copy of array = "<<std::endl;
    std::cout<<(*test2)<<std::endl;

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
    buffer_ext->WriteDataBuffer();
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

    CL_ERROR_TRY
    //buffer_ext->WriteDimensionBuffer();
    fFFTKernel->setArg(1, *( buffer_ext->GetDimensionBuffer() ) );
    CL_ERROR_CATCH

    CL_ERROR_TRY
    fFFTKernel->setArg(4, *( buffer_ext->GetDataBuffer() ) );
    CL_ERROR_CATCH

    //determine the largest global worksize
    fMaxNWorkItems = 0;
    unsigned int n_global;
    //now run the FFT stages (update the twiddle/perm at each stage)
    for(unsigned int D=0; D<NDIM; D++)
    {
        //compute number of 1d fft's needed (n-global)
        unsigned int n_global = 1;
        for(unsigned int i = 0; i < NDIM; i++){ if (i != D){n_global *= dim[i];} }
        std::cout<<"number of FFTs needed:"<<n_global<<std::endl;
        //pad out n-global to be a multiple of the n-local
        unsigned int nDummy = fNLocal - (n_global % fNLocal);
        if(nDummy == fNLocal){ nDummy = 0; }
        n_global += nDummy;
        if (fMaxNWorkItems < n_global){ fMaxNWorkItems = n_global; }

        std::cout<<"D = "<<D<<" dim = "<<dim[D]<<" n local = "<<fNLocal<<" n_global "<<n_global<<" ndummy = "<<nDummy<<std::endl;

        cl::NDRange global(n_global);
        cl::NDRange local(fNLocal);

        //set the arguments which are updated at each stage
        fFFTKernel->setArg(0, D);
        //std::cout<<"flag0"<<std::endl;


        //std::cout<<"flag1"<<std::endl;

        CL_ERROR_TRY
        //fTwiddle[D].AsExtension< MHO_OpenCLNDArrayBuffer< twiddle_type > >()->WriteDataBuffer();
        fFFTKernel->setArg(2, *( fTwiddle[D].AsExtension< MHO_OpenCLNDArrayBuffer< twiddle_type > >()->GetDataBuffer() ) );
        CL_ERROR_CATCH
        //std::cout<<"flag2"<<std::endl;

        CL_ERROR_TRY
        //fPerm[D].AsExtension< MHO_OpenCLNDArrayBuffer< permutation_array_type > >()->WriteDataBuffer();
        fFFTKernel->setArg(3, *( fPerm[D].AsExtension< MHO_OpenCLNDArrayBuffer< permutation_array_type > >()->GetDataBuffer() ) );
        CL_ERROR_CATCH
        //std::cout<<"flag3"<<std::endl;


        //std::cout<<"flag4"<<std::endl;

        //now enqueue the kernel
        CL_ERROR_TRY
        MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueNDRangeKernel(*fFFTKernel,
                                                                         cl::NullRange,
                                                                         global,
                                                                         local);
        CL_ERROR_CATCH

        //force it to finish
        MHO_OpenCLInterface::GetInstance()->GetQueue().finish();

        //get the results (move this out of loop)
        buffer_ext->ReadDataBuffer();
        MHO_OpenCLInterface::GetInstance()->GetQueue().finish();

        std::cout<<"----------"<<std::endl;
        std::cout<<(*test)<<std::endl;
        

    }

    //now do an FFT on the CPU to check we get the same thing
    auto fft_engine = new MHO_MultidimensionalFastFourierTransform< test_table_type >();
    //no do IFFT pass on all axes
    fft_engine->SetForward();
    fft_engine->SetArgs(test2);
    fft_engine->DisableAxisLabelTransformation();
    //fft_engine->SelectAllAxes();

    fft_engine->DeselectAllAxes();
    fft_engine->SelectAxis(0);
    fft_engine->Initialize();
    fft_engine->Execute();

    std::cout<<"alternate path:"<<std::endl;

    std::cout<<"***************"<<std::endl;
    std::cout<<(*test2)<<std::endl;
    fft_engine->DeselectAllAxes();
    fft_engine->SelectAxis(1);
    fft_engine->Initialize();
    fft_engine->Execute();


    std::cout<<"***************"<<std::endl;
    std::cout<< *test2 << std::endl;


    //compute the difference between the results
    double delta = 0;
    for(std::size_t i=0; i<test->GetSize(); i++)
    {
        delta += std::abs( (*test)[i] - (*test2)[i] );
    }
    std::cout<<"delta = "<<delta<<std::endl;


    //clean up
    delete fFFTKernel;
    //delete fft_engine;

    return 0;
}
