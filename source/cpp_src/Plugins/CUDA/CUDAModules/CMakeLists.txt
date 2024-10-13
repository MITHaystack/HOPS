hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

hops_external_include_directories(${CUDA_INCLUDE_DIRS})

set(CUDAModules_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vector_sum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vector_sbp_reduce.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vector_mbp_reduce.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MBDelaySearchCUDA.hh
)

set(CUDAModules_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vector_sum.cu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vector_sbp_reduce.cu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vector_mbp_reduce.cu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MBDelaySearchCUDA.cc
)
# 
# #disable C++11 for this directory, because only the newest 7+ CUDA compilers support it
# set( CMAKE_C_FLAGS "-Wall -Wextra" )
# set( CMAKE_CXX_FLAGS "-Wall -Wextra -pthread" )
# set( CUDA_NVCC_FLAGS "--ptxas-options=-v -D_FORCE_INLINES --compiler-options '-fPIC' -shared")

#list of c++ libraries
set(CUDAMODULES_LIBS MHO_Message MHO_Math MHO_Utilities MHO_Operators MHO_Containers MHO_Calibration)

cuda_add_library(MHO_CUDAModules SHARED ${CUDAModules_SOURCEFILES} )
target_link_libraries (MHO_CUDAModules ${CUDAMODULES_LIBS} ${CUDA_LIBRARIES} ${CUDA_CUFFT_LIBRARIES} )

hops_install_headers( ${CUDAModules_HEADERFILES} )
hops_install_libraries( MHO_CUDAModules )
