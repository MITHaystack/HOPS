#list header files #############################################################

set (HCORE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkMeta.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkNamed.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkUnits.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkTimer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkTokenizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkTimeStampConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkMultiTypeMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkInterval.hh
)

#list source files #############################################################

set( HCORE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HkTimer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HkNamed.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HkTimeStampConverter.cc
)

#declare header paths ##########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(HCORELIBS HInterface)

add_library (HCore SHARED ${HCORE_SOURCEFILES})
target_link_libraries (HCore ${HCORE_LIBS})

hops_install_headers (${HCORE_HEADERFILES})
hops_install_libraries (HCore)
