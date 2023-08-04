#include "MHO_OpenCLInterface.hh"

#include <stdlib.h>
#include <sstream>

#include "MHO_OpenCLConfig.hh"

#ifndef DEFAULT_KERNEL_DIR
#define DEFAULT_KERNEL_DIR "."
#endif /* !DEFAULT_KERNEL_DIR */

namespace hops
{

MHO_OpenCLInterface* MHO_OpenCLInterface::fOpenCLInterface = 0;

MHO_OpenCLInterface::MHO_OpenCLInterface()
{
    InitializeOpenCL();
}

MHO_OpenCLInterface::~MHO_OpenCLInterface()
{
    for (std::vector<cl::CommandQueue*>::iterator it=fQueues.begin();it!=fQueues.end();++it)
    if (*it) delete *it;
}

/**
* Interface to accessing MHO_OpenCLInterface.
*/
MHO_OpenCLInterface* MHO_OpenCLInterface::GetInstance()
{
    if (fOpenCLInterface == 0){fOpenCLInterface = new MHO_OpenCLInterface();}
    return fOpenCLInterface;
}

/**
* Queries the host for available OpenCL platforms, and constructs a Context.
*/
void MHO_OpenCLInterface::InitializeOpenCL()
{
    // Disable CUDA caching, since it doesn't check if included .cl files have
    // changed
    setenv("CUDA_CACHE_DISABLE", "1", 1);

    // Get available platforms
    cl::Platform::get(&fPlatforms);

    // Select the default platform and create a context using this platform and
    // the GPU
    cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM,
    (cl_context_properties)(fPlatforms[HOPS_OPENCL_PLATFORM])(),
    0};

    unsigned int deviceType = HOPS_OPENCL_DEVICE_TYPE;


    if(deviceType == 0) //we have a GPU
    {
        fContext = new cl::Context( CL_DEVICE_TYPE_GPU, cps);
    }

    if(deviceType == 1) //we have a CPU
    {
        fContext = new cl::Context( CL_DEVICE_TYPE_CPU, cps);
    }

    if(deviceType == 2) //we have an accelerator device
    {
        fContext = new cl::Context( CL_DEVICE_TYPE_ACCELERATOR, cps);
    }

    CL_VECTOR_TYPE<cl::Device> availableDevices = fContext->getInfo<CL_CONTEXT_DEVICES>();

    fCLDeviceID = HOPS_DEFAULT_GPU_ID;
    fDevices.clear();
    fDevices = availableDevices;
    fQueues.resize(fDevices.size(),NULL);

    fKernelPath = DEFAULT_KERNEL_DIR;

     FillErrorCodeMaps();
}

/**
* Selects a device for use in OpenCL calculations.
*/
void MHO_OpenCLInterface::SetGPU(unsigned int i)
{
    if (i>=fDevices.size())
    {
        msg_warn("opencl", "Cannot set GPU device to ID # "<<i<<", since there are only "<<fDevices.size()<<" devices available." << eom );
        return;
    }
    #ifdef HOPS_USE_DOUBLE_PRECISION
        cl::STRING_CLASS extensions;
        fDevices[i].getInfo(CL_DEVICE_EXTENSIONS, &extensions);
        if ((extensions.find("cl_khr_fp64") == std::string::npos) &&
        (extensions.find("cl_amd_fp64") == std::string::npos))
        {
            msg_warn("opencl", "Cannot set GPU device to ID # "<<i<<", since it does not support double precision (and this program was built with double precision enabled)." << eom );
            return;
        }

        cl::STRING_CLASS name;
        fDevices[i].getInfo(CL_DEVICE_NAME, &name);
        msg_info("opencl", "Setting GPU device to ID # "<<i<<" ("<<name<<") of "<<fDevices.size()<<" available devices (double precision supported). "<< eom);
    #else
        cl::STRING_CLASS name;
        fDevices[i].getInfo(CL_DEVICE_NAME, &name);
        msg_info("opencl", "Setting GPU device to ID # "<<i<<" ("<<name<<") of "<<fDevices.size()<<" available devices. "<< eom);
    #endif /* HOPS_USE_DOUBLE_PRECISION */

    fCLDeviceID = i;
}

cl::CommandQueue& MHO_OpenCLInterface::GetQueue(int i) const
{
    int deviceID = (i == -1 ? fCLDeviceID : i);
    if (!fQueues.at(deviceID))
    fQueues[deviceID] = new cl::CommandQueue(GetContext(),GetDevice());
    return *fQueues[deviceID];
}


void MHO_OpenCLInterface::FillErrorCodeMaps()
{
    //straight from the OpenCL header (cl.h)
    fOpenCLError2CodeMap["CL_SUCCESS"]=0;
    fOpenCLError2CodeMap["CL_DEVICE_NOT_FOUND"]=-1;
    fOpenCLError2CodeMap["CL_DEVICE_NOT_AVAILABLE"]=-2;
    fOpenCLError2CodeMap["CL_COMPILER_NOT_AVAILABLE"]=-3;
    fOpenCLError2CodeMap["CL_MEM_OBJECT_ALLOCATION_FAILURE"]=-4;
    fOpenCLError2CodeMap["CL_OUT_OF_RESOURCES"]=-5;
    fOpenCLError2CodeMap["CL_OUT_OF_HOST_MEMORY"]=-6;
    fOpenCLError2CodeMap["CL_PROFILING_INFO_NOT_AVAILABLE"]=-7;
    fOpenCLError2CodeMap["CL_MEM_COPY_OVERLAP"]=-8;
    fOpenCLError2CodeMap["CL_IMAGE_FORMAT_MISMATCH"]=-9;
    fOpenCLError2CodeMap["CL_IMAGE_FORMAT_NOT_SUPPORTED"]=-10;
    fOpenCLError2CodeMap["CL_BUILD_PROGRAM_FAILURE"]=-11;
    fOpenCLError2CodeMap["CL_MAP_FAILURE"]=-12;

    #ifdef CL_VERSION_1_1
    fOpenCLError2CodeMap["CL_MISALIGNED_SUB_BUFFER_OFFSET"]=-13;
    fOpenCLError2CodeMap["CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST"]=-14;
    #endif

    #ifdef CL_VERSION_1_2
    fOpenCLError2CodeMap["CL_COMPILE_PROGRAM_FAILURE"]=-15;
    fOpenCLError2CodeMap["CL_LINKER_NOT_AVAILABLE"]=-16;
    fOpenCLError2CodeMap["CL_LINK_PROGRAM_FAILURE"]=-17;
    fOpenCLError2CodeMap["CL_DEVICE_PARTITION_FAILED"]=-18;
    fOpenCLError2CodeMap["CL_KERNEL_ARG_INFO_NOT_AVAILABLE"]=-19;
    #endif

    fOpenCLError2CodeMap["CL_INVALID_VALUE"]=-30;
    fOpenCLError2CodeMap["CL_INVALID_DEVICE_TYPE"]=-31;
    fOpenCLError2CodeMap["CL_INVALID_PLATFORM"]=-32;
    fOpenCLError2CodeMap["CL_INVALID_DEVICE"]=-33;
    fOpenCLError2CodeMap["CL_INVALID_CONTEXT"]=-34;
    fOpenCLError2CodeMap["CL_INVALID_QUEUE_PROPERTIES"]=-35;
    fOpenCLError2CodeMap["CL_INVALID_COMMAND_QUEUE"]=-36;
    fOpenCLError2CodeMap["CL_INVALID_HOST_PTR"]=-37;
    fOpenCLError2CodeMap["CL_INVALID_MEM_OBJECT"]=-38;
    fOpenCLError2CodeMap["CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"]=-39;
    fOpenCLError2CodeMap["CL_INVALID_IMAGE_SIZE"]=-40;
    fOpenCLError2CodeMap["CL_INVALID_SAMPLER"]=-41;
    fOpenCLError2CodeMap["CL_INVALID_BINARY"]=-42;
    fOpenCLError2CodeMap["CL_INVALID_BUILD_OPTIONS"]=-43;
    fOpenCLError2CodeMap["CL_INVALID_PROGRAM"]=-44;
    fOpenCLError2CodeMap["CL_INVALID_PROGRAM_EXECUTABLE"]=-45;
    fOpenCLError2CodeMap["CL_INVALID_KERNEL_NAME"]=-46;
    fOpenCLError2CodeMap["CL_INVALID_KERNEL_DEFINITION"]=-47;
    fOpenCLError2CodeMap["CL_INVALID_KERNEL"]=-48;
    fOpenCLError2CodeMap["CL_INVALID_ARG_INDEX"]=-49;
    fOpenCLError2CodeMap["CL_INVALID_ARG_VALUE"]=-50;
    fOpenCLError2CodeMap["CL_INVALID_ARG_SIZE"]=-51;
    fOpenCLError2CodeMap["CL_INVALID_KERNEL_ARGS"]=-52;
    fOpenCLError2CodeMap["CL_INVALID_WORK_DIMENSION"]=-53;
    fOpenCLError2CodeMap["CL_INVALID_WORK_GROUP_SIZE"]=-54;
    fOpenCLError2CodeMap["CL_INVALID_WORK_ITEM_SIZE"]=-55;
    fOpenCLError2CodeMap["CL_INVALID_GLOBAL_OFFSET"]=-56;
    fOpenCLError2CodeMap["CL_INVALID_EVENT_WAIT_LIST"]=-57;
    fOpenCLError2CodeMap["CL_INVALID_EVENT"]=-58;
    fOpenCLError2CodeMap["CL_INVALID_OPERATION"]=-59;
    fOpenCLError2CodeMap["CL_INVALID_GL_OBJECT"]=-60;
    fOpenCLError2CodeMap["CL_INVALID_BUFFER_SIZE"]=-61;
    fOpenCLError2CodeMap["CL_INVALID_MIP_LEVEL"]=-62;
    fOpenCLError2CodeMap["CL_INVALID_GLOBAL_WORK_SIZE"]=-63;

    #ifdef CL_VERSION_1_1
    fOpenCLError2CodeMap["CL_INVALID_PROPERTY"]=-64;
    #endif

    #ifdef CL_VERSION_1_2
    fOpenCLError2CodeMap["CL_INVALID_IMAGE_DESCRIPTOR"]=-65;
    fOpenCLError2CodeMap["CL_INVALID_COMPILER_OPTIONS"]=-66;
    fOpenCLError2CodeMap["CL_INVALID_LINKER_OPTIONS"]=-67;
    fOpenCLError2CodeMap["CL_INVALID_DEVICE_PARTITION_COUNT"]=-68;
    #endif

    #ifdef CL_VERSION_2_0
    fOpenCLError2CodeMap["CL_INVALID_PIPE_SIZE"]=-69;
    fOpenCLError2CodeMap["CL_INVALID_DEVICE_QUEUE"]=-70;
    #endif

    #ifdef CL_VERSION_2_2
    fOpenCLError2CodeMap["CL_INVALID_SPEC_ID"]=-71;
    fOpenCLError2CodeMap["CL_MAX_SIZE_RESTRICTION_EXCEEDED"]=-72;
    #endif

    for(auto it = fOpenCLError2CodeMap.begin(); it != fOpenCLError2CodeMap.end(); it++)
    {
        auto message = it->first;
        auto code = it->second;
        fOpenCLCode2ErrorMap[code] = message;
    }

}

}//end of namespace



