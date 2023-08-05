#define __CL_ENABLE_EXCEPTIONS

#ifdef HOPS_OPENCL_VERSION2
    #include <CL/cl2.hpp>
#else 
    #include <CL/cl.hpp>
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

int main()
{
    // ifdef-guard header
    std::cout<<"#ifndef HOPS_DEFINES_H"<<std::endl;
    std::cout<<"#define HOPS_DEFINES_H"<<std::endl;

    // Set up list of desired extensions
    std::vector<std::string> desired_extensions;
    desired_extensions.push_back(((std::string)"cl_khr_fp64"));
    desired_extensions.push_back(((std::string)"cl_amd_fp64"));

    // Create flag for double precision support
    bool double_precision = false;

    // Default GPU to use (if more than one are present)
    unsigned int defaultDeviceID = 0;

    // Get available platforms
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    std::cout<<"/* Generated using platform #"<<HOPS_OPENCL_PLATFORM<<" out of "<<platforms.size()<<" platforms. */"<<std::endl;
    std::cout<<std::endl;

    // Select the default platform and create a context using this platform and the GPU
    cl_context_properties cps[3] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platforms[HOPS_OPENCL_PLATFORM])(),
        0
    };

    unsigned int deviceType = HOPS_OPENCL_DEVICE_TYPE;

    cl::Context* context = NULL;

    if(deviceType == 0) //we have a GPU
    {
        context = new cl::Context( CL_DEVICE_TYPE_GPU, cps);
    }

    if(deviceType == 1) //we have a CPU
    {
        context = new cl::Context( CL_DEVICE_TYPE_CPU, cps);
    }

    if(deviceType == 2) //we have an accelerator device
    {
        context = new cl::Context( CL_DEVICE_TYPE_ACCELERATOR, cps);
    }

    // Get a list of devices on this platform
    std::vector<cl::Device> devices = context->getInfo<CL_CONTEXT_DEVICES>();

    std::cout<<"/*"<<std::endl;
    std::cout<<" Number of available platforms on this machine: "<<platforms.size()<<std::endl;
    std::cout<<" Available devices on platform #"<<HOPS_OPENCL_PLATFORM<<std::endl;
    for (unsigned int i=0;i<devices.size();i++)
    {
        std::cout<<" Device # "<<i<<std::endl;
        std::cout<<"   Name:           "<<devices[i].getInfo<CL_DEVICE_NAME>()<<std::endl;
        std::cout<<"   Vendor:         "<<devices[i].getInfo<CL_DEVICE_VENDOR>()<<std::endl;
        std::cout<<"   Profile:        "<<devices[i].getInfo<CL_DEVICE_PROFILE>()<<std::endl;
        std::cout<<"   Version:        "<<devices[i].getInfo<CL_DEVICE_VERSION>()<<std::endl;
        std::cout<<"   Driver Version: "<<devices[i].getInfo<CL_DRIVER_VERSION>()<<std::endl;
        std::cout<<"   Extensions:     "<<devices[i].getInfo<CL_DEVICE_EXTENSIONS>()<<std::endl;
        std::cout<<""<<std::endl;
    }
    std::cout<<"*/ \n"<<std::endl;

    std::stringstream options;

    for (unsigned int i=0;i<devices.size();i++)
    {
        std::string device_ext_list = devices[i].getInfo<CL_DEVICE_EXTENSIONS>();

        std::istringstream iss(device_ext_list);
        std::vector<std::string> device_extensions;
        std::copy(std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>(),
        std::back_inserter<std::vector<std::string> >(device_extensions) );

        for (unsigned int j=0;j<device_extensions.size();j++)
        {
            for (unsigned int k=0;k<desired_extensions.size();k++)
            {
                if (device_extensions[j] == desired_extensions[k])
                {
                    std::cout<<"#pragma OPENCL EXTENSION "
                    <<device_extensions[j]
                    <<" : enable"<<std::endl;
                    desired_extensions[k] = "NULL";
                    if (k==0 || k==1)
                    {
                        if (k==1)
                        options << "-DAMD";
                        double_precision = true;
                        defaultDeviceID = i;
                    }
                }
            }
        }
    }

    // Create a new devices vector with only the default one in it
    cl::Device defaultDevice = devices[defaultDeviceID];
    devices.clear();
    devices.push_back(defaultDevice);

    if (double_precision)
    {
        std::cout<<"#define cl_double double"<<std::endl;
        std::cout<<"#define cl_double2 double2"<<std::endl;
        std::cout<<"#define cl_double4 double4"<<std::endl;
        std::cout<<"#define cl_double8 double8"<<std::endl;
        std::cout<<"#define cl_double16 double16"<<std::endl;

        std::cout<<"#define CL_TYPE double"<<std::endl;
        std::cout<<"#define CL_TYPE2 double2"<<std::endl;
        std::cout<<"#define CL_TYPE4 double4"<<std::endl;
        std::cout<<"#define CL_TYPE8 double8"<<std::endl;
        std::cout<<"#define CL_TYPE16 double16"<<std::endl;
    }
    else
    {
        std::cout<<"#define cl_float float"<<std::endl;
        std::cout<<"#define cl_float float2"<<std::endl;
        std::cout<<"#define cl_float float4"<<std::endl;
        std::cout<<"#define cl_float float8"<<std::endl;
        std::cout<<"#define cl_float float16"<<std::endl;

        std::cout<<"#define CL_TYPE float"<<std::endl;
        std::cout<<"#define CL_TYPE2 float2"<<std::endl;
        std::cout<<"#define CL_TYPE4 float4"<<std::endl;
        std::cout<<"#define CL_TYPE8 float8"<<std::endl;
        std::cout<<"#define CL_TYPE16 float16"<<std::endl;
    }

    #ifndef HOPS_OPENCL_CPU
        if (!double_precision && platforms[0].getInfo<CL_PLATFORM_NAME>()=="Apple")
        {
            // we must recast sqrt() to take only floats
            std::cout<<"#define SQRT(x) sqrt((float)(x))"<<std::endl;
            std::cout<<"#define LOG(x) log((float)(x))"<<std::endl;
        }
        else
    #endif
        {
            std::cout<<"#define SQRT(x) sqrt(x)"<<std::endl;
            std::cout<<"#define LOG(x) log(x)"<<std::endl;
        }

    // complete ifdef-guard header
    std::cout<<""<<std::endl;
    std::cout<<"#endif /* HOPS_DEFINES_H */"<<std::endl;

    delete context;

    if (double_precision){return defaultDeviceID;}
    else{return 255;}
}
