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
}
