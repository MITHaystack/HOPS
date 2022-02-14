#list header files #############################################################
set (CONTAINER_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayMath.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayWrapper_0.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayWrapper_1.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayTypeCastFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_AxisPack.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ScalarContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VectorContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TableContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Axis.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VisibilityType.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Visibilities.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelizedVisibilities.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_StationCoordinates.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SerializableObjectFactory.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ClassIdentityMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerDictionary.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerJSON.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerFileInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerLibrary.hh
)

set( CONTAINER_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ContainerDictionary.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ContainerLibrary.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ContainerFileInterface.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#install library (headers only) ################################################
set(CONTAINER_LIBS MHO_Message MHO_Utilities)

add_library(MHO_Containers SHARED ${CONTAINER_SOURCEFILES})
target_link_libraries(MHO_Containers ${CONTAINER_LIBS})

hops_install_headers (${CONTAINER_HEADERFILES})
hops_install_libraries (MHO_Containers)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
