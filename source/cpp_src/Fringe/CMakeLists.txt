#list header files #############################################################
set (FRINGE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NormFX.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DelayRate.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MBDelaySearch.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_InterpolateFringePeak.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ComputePlotData.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DelayModel.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BasicFringeDataConfiguration.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BasicFringeInfo.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexInfoExtractor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_InitialFringeInfo.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringePlotInfo.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringeFitter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BasicFringeUtilities.hh

)

#list source files #############################################################
set( FRINGE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_NormFX.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DelayRate.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MBDelaySearch.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_InterpolateFringePeak.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ComputePlotData.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DelayModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BasicFringeDataConfiguration.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BasicFringeInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexInfoExtractor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_InitialFringeInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_FringePlotInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BasicFringeUtilities.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(FRINGE_LIBS MHO_Message MHO_Math MHO_Utilities MHO_Operators MHO_Containers MHO_Calibration)

#compile and install library ###################################################
add_library (MHO_Fringe SHARED ${FRINGE_SOURCEFILES})
target_link_libraries (MHO_Fringe ${FRINGE_LIBS})

hops_install_headers (${FRINGE_HEADERFILES})
hops_install_libraries (MHO_Fringe )
# 
# if (HOPS_ENABLE_TEST)
#     add_subdirectory(test)
# endif (HOPS_ENABLE_TEST)
