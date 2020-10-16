#list header files #############################################################
#add an option to turn on/off debug messages, off by default
option(ENABLE_DEBUG_MSG "Enable passing of debug messages" OFF)

set (HKMESSAGE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkMessenger.hh
)

#list source files #############################################################

set( HKMESSAGE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HkMessenger.cc
)

#declare header paths ##########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(HKMESSAGE_LIBS "")

if(ENABLE_DEBUG_MSG)
    add_cflag(HOPS_ENABLE_DEBUG_MSG)
endif(ENABLE_DEBUG_MSG)

add_library (HkMessage SHARED ${HKMESSAGE_SOURCEFILES})
target_link_libraries (HkMessage ${HKMESSAGE_LIBS})

hops_install_headers (${HKMESSAGE_HEADERFILES})
hops_install_libraries (HkMessage )
