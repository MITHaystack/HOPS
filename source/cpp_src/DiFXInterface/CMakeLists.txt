#list header files #############################################################
set (DIFXINTERFACE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXScanFileSet.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXVisibilityRecord.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXInputProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXScanProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXVisibilityProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXBaselineProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXPCalProcessor.hh
    #${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXVexStripper.hh
)

#list source files #############################################################
set( DIFXINTERFACE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXScanProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXInputProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXVisibilityProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXBaselineProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXPCalProcessor.cc
    #${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXVexStripper.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(DIFXINTERFACE_LIBS MHO_Message MHO_Utilities MHO_Containers)

# we use pkg-config to find difxio
find_package(PkgConfig) 
pkg_search_module(DIFXIO REQUIRED difxio IMPORTED_TARGET)
hops_external_include_directories (${DIFXIO_INCLUDE_DIRS})

# list(APPEND DIFXINTERFACE_LIBS ${DIFXIO_LDFLAGS})

add_library (MHO_DiFXInterface SHARED ${DIFXINTERFACE_SOURCEFILES})
message(STATUS "DIFXIO LDFLAGS = ${DIFXIO_LDFLAGS}")
message(STATUS "DIFXIO LIBS = ${DIFIO_LINK_LIBRARIES}")

target_link_libraries(MHO_DiFXInterface ${DIFXINTERFACE_LIBS} PkgConfig::DIFXIO)

hops_install_headers (${DIFXINTERFACE_HEADERFILES})


#FIXME WHAT IS GOING ON HERE?
#WHY DO WE NEED TO INSTALL THE LIBRARY AS A FILE IN ORDER 
#TO PRESERVE THE INTERNAL LINK SYMBOLS TO DIFXIO? 
#FOR EXAMPLE THE FOLLOWING COMMAND:
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libMHO_DiFXInterface.so DESTINATION ${LIB_INSTALL_DIR})
#PRODUCES:
# ldd ./libMHO_DiFXInterface.so 
#    linux-vdso.so.1 (0x00007fffdcc97000)
#    libMHO_Utilities.so => /home/barrettj/work/projects/hops-git/x86_64-4.00/lib/libMHO_Utilities.so (0x00007f233da06000)
#    ....
#    libdifxio.so.0 => /home/barrettj/software/difx-2.6.2/lib/libdifxio.so.0 (0x00007f233d9c5000)
#    ....
#BUT USING hops_install_libraries(), OR ANY OTHER CMAKE install(TARGETS...) COMMAND:
#PRODUCES:
# ldd ./libMHO_DiFXInterface.so 
# 	linux-vdso.so.1 (0x00007ffd155f9000)
# 	libMHO_Utilities.so => /home/barrettj/work/projects/hops-git/x86_64-4.00/lib/libMHO_Utilities.so (0x00007f3c369de000)
# 	....
# 	libdifxio.so.0 => not found
#   ....

add_subdirectory(applications)

hops_install_executables( ${DIFXIO_DEP_SOURCE_BASENAMES} )