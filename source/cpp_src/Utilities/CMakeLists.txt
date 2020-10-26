#list header files #############################################################

set (UTILITIES_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMeta.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHONamed.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOUnits.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOTimer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOTokenizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOTimeStampConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMultiTypeMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOInterval.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOIntervalLabel.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOIntervalLabelTree.hh
)

#list source files #############################################################

set( UTILITIES_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOTimer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHONamed.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOTimeStampConverter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOIntervalLabel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOIntervalLabelTree.cc
)

#declare header paths ##########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(UTILITIES_LIBS "")

add_library (MHOUtilities SHARED ${UTILITIES_SOURCEFILES})
target_link_libraries (MHOUtilities ${UTILITIES_LIBS})

hops_install_headers (${UTILITIES_HEADERFILES})
hops_install_libraries (MHOUtilities)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)

