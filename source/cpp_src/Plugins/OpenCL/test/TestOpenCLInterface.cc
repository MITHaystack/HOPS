#include "MHO_Message.hh"

#include <cmath>
#include <iomanip>
#include <iostream>

#include "MHO_OpenCLInterface.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    unsigned int ndev = MHO_OpenCLInterface::GetInstance()->GetNumberOfDevices();
    std::cout << " Number of available devices on current platform #" << HOPS_OPENCL_PLATFORM << " is : " << ndev << std::endl;
    std::cout << std::endl;
    for(unsigned int i = 0; i < ndev; i++)
    {
        cl::Device dev = MHO_OpenCLInterface::GetInstance()->GetDevice();
        std::cout << " Device # " << i << std::endl;
        std::cout << "   Name:           " << dev.getInfo< CL_DEVICE_NAME >() << std::endl;
        std::cout << "   Vendor:         " << dev.getInfo< CL_DEVICE_VENDOR >() << std::endl;
        std::cout << "   Profile:        " << dev.getInfo< CL_DEVICE_PROFILE >() << std::endl;
        std::cout << "   Version:        " << dev.getInfo< CL_DEVICE_VERSION >() << std::endl;
        std::cout << "   Driver Version: " << dev.getInfo< CL_DRIVER_VERSION >() << std::endl;
        std::cout << "   OpenCL Version:     " << dev.getInfo< CL_DEVICE_OPENCL_C_VERSION >() << std::endl;
        std::cout << "   Global Memory Size:     " << dev.getInfo< CL_DEVICE_GLOBAL_MEM_SIZE >() << std::endl;
        std::cout << "   Local Memory Size:     " << dev.getInfo< CL_DEVICE_LOCAL_MEM_SIZE >() << std::endl;
        std::cout << "   Max Clock Frequency:     " << dev.getInfo< CL_DEVICE_MAX_CLOCK_FREQUENCY >() << std::endl;
        std::cout << "   Max Compute Units:     " << dev.getInfo< CL_DEVICE_MAX_COMPUTE_UNITS >() << std::endl;
        std::cout << "   Max Const Args:     " << dev.getInfo< CL_DEVICE_MAX_CONSTANT_ARGS >() << std::endl;
        std::cout << "   Max Const Buffer Size:     " << dev.getInfo< CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE >() << std::endl;
        std::cout << "   Max Memory Alloc Size:     " << dev.getInfo< CL_DEVICE_MAX_MEM_ALLOC_SIZE >() << std::endl;
        std::cout << "   Max Parameter Size:     " << dev.getInfo< CL_DEVICE_MAX_PARAMETER_SIZE >() << std::endl;
        std::cout << "   Max Workgroup size:     " << dev.getInfo< CL_DEVICE_MAX_WORK_GROUP_SIZE >() << std::endl;
        std::cout << "   Extensions:     " << dev.getInfo< CL_DEVICE_EXTENSIONS >() << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
