if(HOPS_USE_HDF5)

find_package(HDF5 COMPONENTS C CXX HL)

if(HDF5_FOUND)

    hops_internal_include_directories()
    hops_external_include_directories(${HDF5_INCLUDE_DIRS})

    #list header files #############################################################
    set (HDF5INTERFACE_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_HDF5TypeCode.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_HDF5ConversionHelpers.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_HDF5Attributes.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_HDF5Datasets.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerHDF5Converter.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_HDF5ConverterDictionary.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_HDF5ContainerFileInterface.hh
    )

    set( HDF5INTERFACE_SOURCEFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_HDF5ContainerFileInterface.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_HDF5ConverterDictionary.cc
    )

    #declare header paths ##########################################################
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

    #set libraries to link agains library (headers only) ###########################
    set(HDF5INTERFACE_LIBS  MHO_Message MHO_Utilities MHO_Containers ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})


    add_library(MHO_HDF5Interface SHARED ${HDF5INTERFACE_SOURCEFILES})
    target_link_libraries(MHO_HDF5Interface ${HDF5INTERFACE_LIBS})

    hops_install_headers (${HDF5INTERFACE_HEADERFILES})
    hops_install_libraries (MHO_HDF5Interface)

    add_subdirectory(applications)

    if (HOPS_ENABLE_TEST)
        add_subdirectory(test)
    endif (HOPS_ENABLE_TEST)

endif(HDF5_FOUND)


endif(HOPS_USE_HDF5)
