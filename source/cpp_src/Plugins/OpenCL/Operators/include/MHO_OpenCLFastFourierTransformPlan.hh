#ifndef MHO_OpenCLFastFourierTransformPlan_HH__
#define MHO_OpenCLFastFourierTransformPlan_HH__

#include "MHO_FastFourierTransform.hh"
#include "MHO_FastFourierTransformWorkspace.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_MultidimensionalFastFourierTransformInterface.hh"

#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"

#include <string>
#include <sstream>
#include <tuple>

namespace hops 
{

class MHO_OpenCLFastFourierTransformPlan
{

    public:

        MHO_OpenCLFastFourierTransformPlan();
        virtual ~MHO_OpenCLFastFourierTransformPlan();

        //sets the dimension of the global array, and size of the
        //dimension associated with this plan 
        //reconfigures the kernel and necessary buffers 
        void Build(std::size_t NDIM, std::size_t N);

        cl::Kernel* GetKernel();//retrieve the kernel
        std::size_t GetNLocal(){return fNLocal;}


    private:
        /* data */

        std::size_t fNDim;
        std::size_t fN;

        std::size_t fNLocal;
        std::size_t fPreferredWorkgroupMultiple;
        cl::Kernel* fFFTKernel;

        void ConstructKernel(const std::string& file_name, 
                             const std::string& kernel_name,
                             const std::string& cflags);

        enum fft_plan_type 
        {
            radix2_strided,
            radix2_strided_const,
            radix2_private,
            radix2_private_const
            // bluestein_strided,
            // bluestein_strided_const,
            // bluestein_private,
            // bluestein_private_const
        }
    
        using kfile_kfunc_cflags = std::tuple< std::string, std::string, std::string >;
        std::map< fft_plan_type, kfile_kfunc_cflags > fSourceCodeMap;

        void BuildCodeList();

        void Clear();
};

}//end of namespace

#endif /* end of include guard: MHO_OpenCLFastFourierTransformPlan_HH__ */
