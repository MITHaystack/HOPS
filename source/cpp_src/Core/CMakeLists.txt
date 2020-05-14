#list header files #############################################################

set (HCORE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HMeta.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HTimer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HTokenizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HTimeStampConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HMultiTypeMap.hh
)

#list source files #############################################################

set( HCORE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTimer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTimeStampConverter.cc
)

#declare header paths ##########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../c_src/Interface/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(HCORELIBS HInterface)

add_library (HCore SHARED ${HCORE_SOURCEFILES})
target_link_libraries (HCore ${HCORE_LIBS})

hops_install_headers (${HCORE_HEADERFILES})
hops_install_libraries (HCore)
