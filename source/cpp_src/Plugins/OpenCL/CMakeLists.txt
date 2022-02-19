set(HOPS_OPENCL_PLATFORM "0" CACHE STRING "OpenCL platform ID number.")
mark_as_advanced(FORCE HOPS_OPENCL_PLATFORM)

set(HOPS_OPENCL_DEVICE_TYPE "0" CACHE STRING "OpenCL device type; GPU=0, CPU=1, Accelerator=2.")
mark_as_advanced(FORCE HOPS_OPENCL_DEVICE_TYPE)

CMAKE_DEPENDENT_OPTION (HOPS_OPENCL_USE_DEPRECATED_API "Use deprecated OpenCL 1.1 API" OFF
HOPS_USE_OPENCL OFF)
mark_as_advanced(FORCE HOPS_OPENCL_USE_DEPRECATED_API)
if (HOPS_OPENCL_USE_DEPRECATED_API)
set (HOPS_OPENCL_CFLAGS ${HOPS_OPENCL_CFLAGS} -DCL_USE_DEPRECATED_OPENCL_1_1_APIS)
endif (HOPS_OPENCL_USE_DEPRECATED_API)

CMAKE_DEPENDENT_OPTION (HOPS_OPENCL_USE_CL_VECTOR "Use deprecated cl::vector in place of std::vector" OFF
HOPS_USE_OPENCL OFF)
mark_as_advanced(FORCE HOPS_OPENCL_USE_CL_VECTOR)
if (HOPS_OPENCL_USE_CL_VECTOR)
set (HOPS_OPENCL_CFLAGS ${HOPS_OPENCL_CFLAGS} -DHOPS_USE_CL_VECTOR)
endif (HOPS_OPENCL_USE_CL_VECTOR)

CMAKE_DEPENDENT_OPTION (HOPS_OPENCL_INTERNAL_1_1 "Use internally packaged OpenCL 1.1 headers" ON
HOPS_USE_OPENCL OFF)
mark_as_advanced(FORCE HOPS_OPENCL_INTERNAL_1_1)
if (HOPS_OPENCL_INTERNAL_1_1)
set (HOPS_OPENCL_TEMP_INCLUDE_DIR_STR \"${CMAKE_CMAKE_CURRENT_SOURCE_DIR_DIR}/API/opencl/1.1\")
string (REPLACE "\"" "" HOPS_OPENCL_TEMP_INCLUDE_DIR_VAR ${HOPS_OPENCL_TEMP_INCLUDE_DIR_STR} )
set (HOPS_OPENCL_CFLAGS ${HOPS_OPENCL_CFLAGS} -isystem ${HOPS_OPENCL_TEMP_INCLUDE_DIR_VAR})
endif (HOPS_OPENCL_INTERNAL_1_1)

CMAKE_DEPENDENT_OPTION (HOPS_OPENCL_INTERNAL_1_2 "Use internally packaged OpenCL 1.2 headers" OFF
HOPS_USE_OPENCL OFF)
mark_as_advanced(FORCE HOPS_OPENCL_INTERNAL_1_2)
if (HOPS_OPENCL_INTERNAL_1_2)
set (HOPS_OPENCL_TEMP_INCLUDE_DIR_STR \"${CMAKE_CMAKE_CURRENT_SOURCE_DIR_DIR}/API/opencl/1.2\")
string (REPLACE "\"" "" HOPS_OPENCL_TEMP_INCLUDE_DIR_VAR ${HOPS_OPENCL_TEMP_INCLUDE_DIR_STR} )
set (HOPS_OPENCL_CFLAGS ${HOPS_OPENCL_CFLAGS} -isystem ${HOPS_OPENCL_TEMP_INCLUDE_DIR_VAR})
endif (HOPS_OPENCL_INTERNAL_1_2)

CMAKE_DEPENDENT_OPTION (HOPS_OPENCL_INTERNAL_2_2 "Use internally packaged OpenCL 2.2 headers" OFF
HOPS_USE_OPENCL OFF)
mark_as_advanced(FORCE HOPS_OPENCL_INTERNAL_2_2)
if (HOPS_OPENCL_INTERNAL_2_2)
set (HOPS_OPENCL_TEMP_INCLUDE_DIR_STR \"${CMAKE_CMAKE_CURRENT_SOURCE_DIR_DIR}/API/opencl/2.2\")
string (REPLACE "\"" "" HOPS_OPENCL_TEMP_INCLUDE_DIR_VAR ${HOPS_OPENCL_TEMP_INCLUDE_DIR_STR} )
set (HOPS_OPENCL_CFLAGS ${HOPS_OPENCL_CFLAGS} -isystem ${HOPS_OPENCL_TEMP_INCLUDE_DIR_VAR})
endif (HOPS_OPENCL_INTERNAL_2_2)

CMAKE_DEPENDENT_OPTION (HOPS_OPENCL_DEBUG "OpenCL verbose debugging mode." ON HOPS_USE_OPENCL OFF)
mark_as_advanced(FORCE HOPS_OPENCL_DEBUG)
if (HOPS_OPENCL_DEBUG)
set(DEBUG_OPENCL_COMPILER_OUTPUT 1)
set(USE_CL_ERROR_TRY_CATCH 1)
endif (HOPS_OPENCL_DEBUG)

CMAKE_DEPENDENT_OPTION (HOPS_ENFORCE_CLFINISH "Force OpenCL kernels/buffer x-fers to execute sequentially (for debug)." ON HOPS_USE_OPENCL OFF)
mark_as_advanced(FORCE HOPS_ENFORCE_CLFINISH)
if (HOPS_ENFORCE_CLFINISH)
set(ENFORCE_CL_FINISH 1)
endif (HOPS_ENFORCE_CLFINISH)

option (HOPS_USE_OPENCL "Use OpenCL via C++ wrapper API." OFF)
if (HOPS_USE_OPENCL)

    message(STATUS "Looking for OpenCL enabled devices ...")

    find_package (OpenCL REQUIRED)

    hops_external_include_directories(${OPENCL_INCLUDE_DIRS})

    if(HOPS_OPENCL_INTERNAL_1_1)
    hops_internal_include_directories(SYSTEM ${CMAKE_CMAKE_CURRENT_SOURCE_DIR_DIR}/API/opencl/1.1)
    endif(HOPS_OPENCL_INTERNAL_1_1)

    if(HOPS_OPENCL_INTERNAL_1_2)
    hops_internal_include_directories(SYSTEM ${CMAKE_CMAKE_CURRENT_SOURCE_DIR_DIR}/API/opencl/1.2)
    endif(HOPS_OPENCL_INTERNAL_1_2)

    if(HOPS_OPENCL_INTERNAL_1_2)
    hops_internal_include_directories(SYSTEM ${CMAKE_CMAKE_CURRENT_SOURCE_DIR_DIR}/API/opencl/1.2)
    endif(HOPS_OPENCL_INTERNAL_1_2)

    hops_add_cxxflag (HOPS_USE_OPENCL)

    get_filename_component(OPENCL_LIBDIR ${OPENCL_LIBRARIES} PATH)

    execute_process(COMMAND ${CMAKE_CXX_COMPILER}
        -D HOPS_OPENCL_PLATFORM=${HOPS_OPENCL_PLATFORM}
        -D HOPS_OPENCL_DEVICE_TYPE=${HOPS_OPENCL_DEVICE_TYPE}
        -o GenerateOpenCLHeader
        -I${OPENCL_INCLUDE_DIRS}
        ${HOPS_OPENCL_CFLAGS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/src/GenerateOpenCLHeader.cc
        -L${OPENCL_LIBDIR} ${OPENCL_LIB_FLAG}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE CL_STATUS_OK
        ERROR_VARIABLE NOT_USED)

        if(CL_STATUS_OK AND NOT CL_STATUS_OK EQUAL 0)
          message(FATAL_ERROR "Could not build the OpenCL header generator." )
        endif()

    execute_process(COMMAND ./GenerateOpenCLHeader
        OUTPUT_FILE ${CMAKE_BINARY_DIR}/hopsocl_defines.h
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE USE_DOUBLE_PRECISION_GPU
        OUTPUT_VARIABLE NOT_USED)

    message(STATUS "GPU double precision result = ${USE_DOUBLE_PRECISION_GPU}")

    if (USE_DOUBLE_PRECISION_GPU STREQUAL "255")
        set(HOPS_DEFAULT_GPU_ID 0)
    else (USE_DOUBLE_PRECISION_GPU STREQUAL "255")
        set(HOPS_USE_DOUBLE_PRECISION 1)
        set(HOPS_DEFAULT_GPU_ID ${USE_DOUBLE_PRECISION_GPU})
    endif (USE_DOUBLE_PRECISION_GPU STREQUAL "255")

    #leave this binary around since it is useful for debugging if things go wrong
    #execute_process(COMMAND rm GenerateOpenCLHeader WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Core/include/MHO_OpenCLConfig.hh.in ${CMAKE_CURRENT_BINARY_DIR}/Core/include/MHO_OpenCLConfig.hh @ONLY)

    set (OPENCLPLUGIN_HEADERFILES
        ${CMAKE_CURRENT_BINARY_DIR}/Core/include/MHO_OpenCLConfig.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/include/MHO_OpenCLHeaderWrapper.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/include/MHO_OpenCLInterface.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/include/MHO_OpenCLKernelBuilder.hh
        #
        ${CMAKE_CURRENT_SOURCE_DIR}/Containers/include/MHO_OpenCLNDArrayBuffer.hh
    )

    #include directories
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core/include)
    hops_internal_include_directories(${CMAKE_CURRENT_BINARY_DIR}/Core/include) #BINARY_DIR includes the generated cl header file
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Math/include)
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Containers/include)

    set (OPENCLPLUGIN_SOURCEFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/src/MHO_OpenCLInterface.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/src/MHO_OpenCLKernelBuilder.cc
    )

    set_property( SOURCE ${OPENCLPLUGIN_SOURCEFILES}
        APPEND PROPERTY COMPILE_DEFINITIONS DEFAULT_KERNEL_DIR="${OPENCL_KERNEL_INSTALL_DIR}"
    )

    add_library (MHO_OpenCLPlugin SHARED ${OPENCLPLUGIN_SOURCEFILES})

    target_link_libraries (MHO_OpenCLPlugin
        MHO_Message
        MHO_Utilities
        MHO_Operators
        MHO_Containers
        ${OPENCL_LIBRARIES}
    )

    hops_install_headers (${OPENCLPLUGIN_HEADERFILES})
    hops_install_libraries (MHO_OpenCLPlugin)

    #install cl header and kernel files

    set (OPENCLPLUGIN_CLFILES
        ${CMAKE_BINARY_DIR}/hopsocl_defines.h
        #
        ${CMAKE_CURRENT_SOURCE_DIR}/Math/cl/MHO_ArrayMath.cl
        ${CMAKE_CURRENT_SOURCE_DIR}/Math/cl/MHO_ComplexMultiply.cl
        ${CMAKE_CURRENT_SOURCE_DIR}/Math/cl/MHO_PointwiseComplexVectorAdd_kernel.cl
        ${CMAKE_CURRENT_SOURCE_DIR}/Math/cl/MHO_PointwiseComplexVectorMultiply_kernel.cl
        ${CMAKE_CURRENT_SOURCE_DIR}/Math/cl/MHO_ZeroComplexArray_kernel.cl
        #
        ${CMAKE_CURRENT_SOURCE_DIR}/Operators/cl/MHO_VectorScale_kernel.cl
    )

    install ( FILES ${OPENCLPLUGIN_CLFILES}  DESTINATION ${OPENCL_KERNEL_INSTALL_DIR} )

    if (HOPS_ENABLE_TEST)
        add_subdirectory(test)
    endif (HOPS_ENABLE_TEST)

endif (HOPS_USE_OPENCL)
