#include "MHO_OpenCLFastFourierTransformPlan.hh"

namespace hops
{

MHO_OpenCLFastFourierTransformPlan::MHO_OpenCLFastFourierTransformPlan()
{
    BuildCodeList();
};

MHO_OpenCLFastFourierTransformPlan::~MHO_OpenCLFastFourierTransformPlan()
{

};

//sets the dimension of the global array, and size of the
//dimension associated with this plan
//reconfigures the kernel and necessary buffers
void MHO_OpenCLFastFourierTransformPlan::Resize(std::size_t NDIM, std::size_t N)
{
    fNDim = NDIM;
    fN = N;

}

void MHO_OpenCLFastFourierTransformPlan::ConstructOpenCLKernel()
{


}

void
MHO_OpenCLFastFourierTransformPlan::ConstructKernel(const std::string& file_name,
                                                    const std::string& kernel_name,
                                                    const std::string& cflags)
{
    std::cout<<"building file: "<<kernel_name << " kernel: "<<kernel_name<<std::endl;
    //Get name of kernel source file
    std::stringstream clFile;
    clFile << MHO_OpenCLInterface::GetInstance()->GetKernelPath() <<
    clFile << "/" << file_name;

    // //create the build flags
    std::stringstream opts;
    opts << cflags;
    opts << " -D FFT_NDIM=" << fNDim;
    // //opts << " -cl-std=CL2.0 ";
    opts << " -I " << MHO_OpenCLInterface::GetInstance()->GetKernelPath();

    //compile the kernel
    MHO_OpenCLKernelBuilder k_builder;
    fFFTKernel = k_builder.BuildKernel(clFile.str(), kernel_name, opts.str() );

    //get n-local
    auto device = MHO_OpenCLInterface::GetInstance()->GetDevice();
    fNLocal = fFFTKernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device);
    fPreferredWorkgroupMultiple = fFFTKernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);
    if(fPreferredWorkgroupMultiple < fNLocal){fNLocal = fPreferredWorkgroupMultiple;}
}



void
MHO_OpenCLFastFourierTransformPlan::BuildCodeList()
{
    // radix2_strided,
    // radix2_strided_const,
    // radix2_private,
    // radix2_private_const,
    // bluestein_strided,
    // bluestein_strided_const,
    // bluestein_private,
    // bluestein_private_const

    fSourceCodeMap.insert(radix2_strided,
        std::make_tuple(
            "MHO_MultidimensionalFastFourierTransformStrided_kernel.cl",
            "MultidimensionalFastFourierTransformStrided_Radix2Stage",
            "")
    );

    fSourceCodeMap.insert(radix2_strided_const,
        std::make_tuple(
            "MHO_MultidimensionalFastFourierTransformStrided_kernel.cl",
            "MultidimensionalFastFourierTransformStrided_Radix2Stage",
            " -D FFT_USE_CONST_MEM ")
    );

    fSourceCodeMap.insert(radix2_private,
        std::make_tuple(
            "MHO_MultidimensionalFastFourierTransformPrivate_kernel.cl",
            "MultidimensionalFastFourierTransformPrivate_Radix2Stage",
            "")
    );

    fSourceCodeMap.insert(radix2_private_const,
        std::make_tuple(
            "MHO_MultidimensionalFastFourierTransformPrivate_kernel.cl",
            "MultidimensionalFastFourierTransformPrivate_Radix2Stage",
            " -D FFT_USE_CONST_MEM ")
    );
}

//destroys kernel and buffers
void Clear()
{
    delete fFFTKernel;
}

}