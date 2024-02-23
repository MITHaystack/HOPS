#ifndef HOPS_OPENCLINTERFACE_DEF
#define HOPS_OPENCLINTERFACE_DEF

#include "MHO_OpenCLConfig.hh"

#ifdef HOPS_USE_CL_VECTOR
    #define __NO_STD_VECTOR // Use cl::vector instead of STL version
    #define CL_VECTOR_TYPE cl::vector
#else
    #define CL_VECTOR_TYPE std::vector
#endif

#define __CL_ENABLE_EXCEPTIONS

#include "MHO_Message.hh"
#include "MHO_TemplateTypenameDeduction.hh"

//shut up the annoying unused variable warnings in cl.hpp for clang/gcc
//by using a system-header wrapper for the opencl headers
#include "MHO_OpenCLHeaderWrapper.hh"
#include <vector>
#include <complex>
#include <string>
#include <map>


template< typename XValueType >
struct MHO_OpenCLTypeMap
{
    using mapped_type = XValueType;
};

#ifdef HOPS_USE_DOUBLE_PRECISION

    #define CL_TYPE cl_double
    #define CL_TYPE2 cl_double2
    #define CL_TYPE4 cl_double4
    #define CL_TYPE8 cl_double8
    #define CL_TYPE16 cl_double16

    //we map doubles to CL_TYPE and complex doubles on to cl_double2
    template<>
    struct MHO_OpenCLTypeMap< double >{using mapped_type = CL_TYPE;};

    template<>
    struct MHO_OpenCLTypeMap< std::complex< double >  >{using mapped_type = CL_TYPE2;};

#else

    #pragma message ("Warning : MHO_OpenCLPlugin is being built with float precision!")
    #define CL_TYPE cl_float
    #define CL_TYPE2 cl_float2
    #define CL_TYPE4 cl_float4
    #define CL_TYPE8 cl_float8
    #define CL_TYPE16 cl_float16

    //we map floats to CL_TYPE and complex floats on to cl_float2
    template<>
    struct MHO_OpenCLTypeMap< float >{using mapped_type = CL_TYPE;};

    template<>
    struct MHO_OpenCLTypeMap< std::complex< float >  >{using mapped_type = CL_TYPE2;};

#endif



//this is necessary on some intel devices
#define ENFORCE_CL_FINISH

//the following are optional defines for debugging

//adds verbose output of kernel build logs
//#define DEBUG_OPENCL_COMPILER_OUTPUT

//add try-catch for opencl exceptions
#define USE_CL_ERROR_TRY_CATCH

#ifdef USE_CL_ERROR_TRY_CATCH
#include <iostream>
#define CL_ERROR_TRY try {
#else
#define CL_ERROR_TRY
#endif

#ifdef USE_CL_ERROR_TRY_CATCH
#define CL_ERROR_CATCH  } catch (cl::Error error) \
                        { \
                            std::cout<<"OpenCL Exception caught: "<<std::endl;\
                            std::cout<<__FILE__<<":"<<__LINE__<<std::endl; \
                            std::cout<<error.what()<<"("<<error.err()<<") = "<<  MHO_OpenCLInterface::GetInstance()->GetErrorMessage(error.err() )  << std::endl; \
                            std::exit(1); \
                        }
#else
#define CL_ERROR_CATCH
#endif



namespace hops
{

    class MHO_OpenCLInterface
    {
        public:

            static MHO_OpenCLInterface* GetInstance();

            cl::Context GetContext() const { return *fContext; }
            CL_VECTOR_TYPE<cl::Device> GetDevices() const { return fDevices; }
            cl::Device GetDevice()  const { return fDevices[fCLDeviceID]; }
            cl::CommandQueue& GetQueue(int i=-1) const;

            unsigned int GetNumberOfDevices() const
            {
                CL_VECTOR_TYPE<cl::Device> availableDevices = fContext->getInfo<CL_CONTEXT_DEVICES>();
                return availableDevices.size();
            };

            void SetGPU(unsigned int i);

            void SetKernelPath(std::string s) { fKernelPath = s; }
            std::string GetKernelPath() const { return fKernelPath; }

            std::string GetErrorMessage(int code){return fOpenCLCode2ErrorMap[code];}

        protected:

            MHO_OpenCLInterface();
            virtual ~MHO_OpenCLInterface();

            void InitializeOpenCL();
            void FillErrorCodeMaps();

            static MHO_OpenCLInterface* fOpenCLInterface;

            std::string fKernelPath;

            CL_VECTOR_TYPE<cl::Platform> fPlatforms;
            CL_VECTOR_TYPE<cl::Device> fDevices;
            unsigned int fCLDeviceID;
            cl::Context* fContext;
            mutable std::vector<cl::CommandQueue*>  fQueues;

            std::map<std::string, int> fOpenCLError2CodeMap;
            std::map<int, std::string> fOpenCLCode2ErrorMap;

    };

}

#endif /*! HOPS_OPENCLINTERFACE_DEF */
