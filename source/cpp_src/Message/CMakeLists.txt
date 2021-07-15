#list header files #############################################################
#add an option to turn on/off debug messages, off by default
option(ENABLE_DEBUG_MSG "Enable passing of debug messages" OFF)

set (MESSAGE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Message.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TestAssertions.hh
)

#list source files #############################################################

set( MESSAGE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Message.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(MESSAGE_LIBS "")

if(ENABLE_DEBUG_MSG)
    hops_add_cxxflag(HOPS_ENABLE_DEBUG_MSG)
endif(ENABLE_DEBUG_MSG)

add_library (MHO_Message SHARED ${MESSAGE_SOURCEFILES})
target_link_libraries (MHO_Message ${MESSAGE_LIBS})

hops_install_headers (${MESSAGE_HEADERFILES})
hops_install_libraries (MHO_Message )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
