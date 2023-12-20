option (HOPS_BUILD_EXTRA_CONTAINERS "Build extended support of a large variety of extra data container types." OFF)

if(HOPS_BUILD_EXTRA_CONTAINERS)
    hops_add_cxxflag(HOPS_BUILD_EXTRA_CONTAINERS)
endif()

#list header files #############################################################
set (CONTAINER_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayMath.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayView.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BidirectionalIndexedIterator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IntervalLabel.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IntervalLabelTree.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Taggable.hh
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IndexLabelInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ObjectTags.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayWrapper_0.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayWrapper_1.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayTypeCastFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_AxisPack.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ScalarContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VectorContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TableContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Axis.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ElementTypes.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_AxisTypes.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BaselineContainers.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_StationContainers.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerDefinitions.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SerializableObjectFactory.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ClassIdentityMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerDictionary.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerJSONConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerFileInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ContainerStore.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ScanDataStore.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Snapshot.hh
)

set( CONTAINER_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IntervalLabel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IntervalLabelTree.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ContainerDictionary.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ContainerFileInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ScanDataStore.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ContainerStore.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Snapshot.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#install library (headers only) ################################################
set(CONTAINER_LIBS MHO_Message MHO_Utilities)

#add compile defines
hops_add_cxxflag("HOPS_SNAPSHOT_DIR=${SNAPSHOT_INSTALL_DIR}")

add_library(MHO_Containers SHARED ${CONTAINER_SOURCEFILES})
target_link_libraries(MHO_Containers ${CONTAINER_LIBS})

hops_install_headers (${CONTAINER_HEADERFILES})
hops_install_libraries (MHO_Containers)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
