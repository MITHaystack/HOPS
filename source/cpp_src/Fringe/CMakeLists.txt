hops_internal_include_directories()
hops_internal_include_directories(../../../extern/cli11)


#list header files #############################################################
set (FRINGE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ComputePlotData.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BasicFringeDataConfiguration.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringeControlInitialization.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BasicFringeInfo.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexInfoExtractor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_InitialFringeInfo.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringePlotInfo.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BasicFringeUtilities.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringeFitter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BasicFringeFitter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IonosphericFringeFitter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_AFileInfoExtractor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_AFileDefinitions.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringeFitterFactory.hh
)

#list source files #############################################################
set( FRINGE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ComputePlotData.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BasicFringeDataConfiguration.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_FringeControlInitialization.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BasicFringeInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexInfoExtractor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_InitialFringeInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_FringePlotInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BasicFringeUtilities.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BasicFringeFitter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IonosphericFringeFitter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_AFileInfoExtractor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_AFileDefinitions.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_FringeFitterFactory.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(FRINGE_LIBS MHO_Message MHO_Math MHO_Utilities MHO_Operators MHO_Containers MHO_Calibration MHO_Control MHO_Initialization)

if(HOPS_USE_CUDA)
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Plugins/CUDA/CUDAModules/include)
    list(APPEND FRINGE_LIBS MHO_CUDAModules ${CUDA_LIBRARIES} ${CUDA_CUFFT_LIBRARIES} )
endif(HOPS_USE_CUDA)


set_property( SOURCE ${FRINGE_SOURCEFILES}
    APPEND PROPERTY COMPILE_DEFINITIONS HOPS_AFILE_FORMAT_DIR="${DATA_INSTALL_DIR}"
)

#compile and install library ###################################################
add_library (MHO_Fringe SHARED ${FRINGE_SOURCEFILES})
target_link_libraries (MHO_Fringe ${FRINGE_LIBS})

hops_install_headers (${FRINGE_HEADERFILES})
hops_install_libraries (MHO_Fringe )

#install hops afio summary format specification
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/format)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
