#ifndef MHO_OPENCLHEADERWRAPPER_DEF
#define MHO_OPENCLHEADERWRAPPER_DEF

#ifdef __clang__
    #pragma clang system_header
#endif
#ifdef __GNUG__
    #pragma GCC system_header
#endif

#if defined __APPLE__
    #ifdef HOPS_OPENCL_VERSION2
        #include <OpenCL/cl2.hpp>
    #else
        #include <OpenCL/cl.hpp>
    #endif
#else
    #ifdef HOPS_OPENCL_VERSION2
        #include <CL/cl2.hpp>
    #else
        #include <CL/cl.hpp>
    #endif
#endif

#endif /*! MHO_OPENCLHEADERWRAPPER_DEF */
