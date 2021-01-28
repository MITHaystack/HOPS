#list header files #############################################################
set (UTILITIES_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Meta.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Named.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Units.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Timer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Tokenizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TimeStampConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultiTypeMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Interval.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IntervalLabel.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IntervalLabelTree.hh
)

#list source files #############################################################

set( UTILITIES_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Timer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Named.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_TimeStampConverter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IntervalLabel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IntervalLabelTree.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(UTILITIES_LIBS "")

add_library (MHO_Utilities SHARED ${UTILITIES_SOURCEFILES})
target_link_libraries (MHO_Utilities ${UTILITIES_LIBS})

hops_install_headers (${UTILITIES_HEADERFILES})
hops_install_libraries (MHO_Utilities)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
