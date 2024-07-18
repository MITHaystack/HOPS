if(HOPS_USE_CUDA)
    include(FindCUDA)
    find_package(CUDA REQUIRED)
    #recurse to subdirs
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/CUDAModules)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif(HOPS_USE_CUDA)
