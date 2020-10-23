#list header files #############################################################
#add an option to turn on/off debug messages, off by default
option(ENABLE_DEBUG_MSG "Enable passing of debug messages" OFF)

set (MESSAGE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMessage.hh
)

#list source files #############################################################

set( MESSAGE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOMessage.cc
)

#declare header paths ##########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(MESSAGE_LIBS "")

if(ENABLE_DEBUG_MSG)
    add_cflag(HOPS_ENABLE_DEBUG_MSG)
endif(ENABLE_DEBUG_MSG)

add_library (MHOMessage SHARED ${MESSAGE_SOURCEFILES})
target_link_libraries (MHOMessage ${MESSAGE_LIBS})

hops_install_headers (${MESSAGE_HEADERFILES})
hops_install_libraries (MHOMessage )
