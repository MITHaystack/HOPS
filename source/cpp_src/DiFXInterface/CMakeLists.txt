include_directories(${EXTERN_HLIBS}/date/include)

#list header files #############################################################
set (DIFXINTERFACE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXScanFileSet.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXVisibilityRecord.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXScanProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXVisibilityProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXBaselineProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXPCalProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXChannelNameConstructor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXOvexPatcher.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXStationCoordBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXTimeUtilities.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXZoomBandRebuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4ChanId.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SkyFreqGrid.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexHelpers.hh
)

#list source files #############################################################
set( DIFXINTERFACE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXScanProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXVisibilityProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXBaselineProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXPCalProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXChannelNameConstructor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXOvexPatcher.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXStationCoordBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXZoomBandRebuilder.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(DIFXINTERFACE_LIBS MHO_Message MHO_Utilities MHO_Vex2JSON MHO_Containers MHO_Operators MHO_MK4Interface)

# NOTE: this library no longer links difxio. The difxio-dependent .input parser
# lives in the GPL-licensed difxinput2json executable (applications/difxinput2json);
# this MIT-licensed code consumes the JSON that program emits instead. As a
# result this library (and difx2hops) are fully relocatable and carry no GPL
# dependency.

add_library (MHO_DiFXInterface SHARED ${DIFXINTERFACE_SOURCEFILES})
target_link_libraries(MHO_DiFXInterface ${DIFXINTERFACE_LIBS})

hops_install_headers (${DIFXINTERFACE_HEADERFILES})
install(TARGETS MHO_DiFXInterface DESTINATION ${LIB_INSTALL_RELDIR})

add_subdirectory(applications)
