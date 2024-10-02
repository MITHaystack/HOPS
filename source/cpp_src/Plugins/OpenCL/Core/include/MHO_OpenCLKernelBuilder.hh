#ifndef MHO_OpenCLKernelBuilder_HH__
#define MHO_OpenCLKernelBuilder_HH__

#include "MHO_OpenCLInterface.hh"
#include <string>

namespace hops
{

class MHO_OpenCLKernelBuilder
{
    public:
        MHO_OpenCLKernelBuilder(){};
        virtual ~MHO_OpenCLKernelBuilder(){};

        cl::Kernel* BuildKernel(std::string SourceFileName, std::string KernelName, std::string BuildFlags = std::string(""));

    protected:
};

} // namespace hops

#endif /*! MHO_OpenCLKernelBuilder_H__ */
