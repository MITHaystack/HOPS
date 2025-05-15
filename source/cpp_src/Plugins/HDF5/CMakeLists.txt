if(HOPS_USE_HDF5)
    #include(FindHDF5)
    #find_package(HDF5 REQUIRED)
    find_package(HDF5 COMPONENTS C CXX HL)
    #recurse to subdirs
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif(HOPS_USE_HDF5)
