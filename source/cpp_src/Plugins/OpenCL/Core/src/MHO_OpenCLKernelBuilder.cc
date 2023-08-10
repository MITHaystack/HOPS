#include "MHO_OpenCLKernelBuilder.hh"

#include <sstream>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "MHO_Message.hh"

#define DEBUG_OPENCL_COMPILER_OUTPUT

namespace hops
{


//calling process is responsible for clean up of the kernel!
cl::Kernel* MHO_OpenCLKernelBuilder::BuildKernel(std::string SourceFileName, std::string KernelName, std::string BuildFlags)
{
    //read in the source code
    std::string sourceCode;
    std::ifstream sourceFile(SourceFileName.c_str());
    sourceCode = std::string(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));

    if(sourceCode.size() < 1)
    {
        msg_warn("opencl", "The kernel file: "<<SourceFileName<<" contains no code."<< eom );
    }

    //Make program of the source code in the context
    cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()+1));
    cl::Program program(MHO_OpenCLInterface::GetInstance()->GetContext(), source);

    //set the build options
    std::stringstream options;
    options.str("");
    options << BuildFlags;
    options << " -I " <<MHO_OpenCLInterface::GetInstance()->GetKernelPath();

    msg_info("opencl", "Attempting to build the kernel found in: " << SourceFileName << eom);

    // Build program for these specific devices
    try
    {
        // use only target device!
        CL_VECTOR_TYPE<cl::Device> devices;
        devices.push_back( MHO_OpenCLInterface::GetInstance()->GetDevice() );
        program.build(devices,options.str().c_str());
    }
    catch (cl::Error error)
    {
        std::cout<<__FILE__<<":"<<__LINE__<<std::endl;
        std::stringstream s;
        s<<"There was an error compiling the kernel: "<<KernelName<<std::endl;
        s<<"In source file: "<<SourceFileName<<std::endl;
        s<<"Here is the information from the OpenCL C++ API:"<<std::endl;
        s<<error.what()<<"("<<error.err()<<")"<<std::endl;
        s<<"Build Status: "<<program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(MHO_OpenCLInterface::GetInstance()->GetDevice())<<std::endl;
        s<<"Build Options:\t"<<program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(MHO_OpenCLInterface::GetInstance()->GetDevice())<<std::endl;
        s<<"Build Log:\t "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(MHO_OpenCLInterface::GetInstance()->GetDevice())<<std::endl;
        std::cout<<s.str()<<std::endl;
        std::exit(1);
    }

    #ifdef DEBUG_OPENCL_COMPILER_OUTPUT
    std::stringstream s;
    s << "Build flags for Kernel "<<KernelName<<" : ";
    s << BuildFlags;
    s << std::endl;
    s << "Build Log for Kernel "<<KernelName<<" :\t ";
    std::stringstream build_log_stream;
    build_log_stream<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(MHO_OpenCLInterface::GetInstance()->GetDevice())<<std::endl;
    std::string build_log;
    build_log = build_log_stream.str();
    #endif

    // Make kernel
    cl_int err_code = -1;
    cl::Kernel* kernel;
    try
    {
        kernel = new cl::Kernel(program, KernelName.c_str(), &err_code);
    }
    catch(cl::Error error)
    {
        std::cout<<"Kernel construction of "<< KernelName <<" failed with error code: "<<error.what()<<": "<<error.err() <<": "<<MHO_OpenCLInterface::GetInstance()->GetErrorMessage(error.err() )<<std::endl;
        if(build_log.size() != 0)
        {
            s << build_log;
            std::cout<<s.str()<<std::endl;
        }
        std::exit(1);
    }

    return kernel;
}



}
