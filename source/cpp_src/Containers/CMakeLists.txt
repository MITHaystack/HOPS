#list header files #############################################################
set (CONTAINER_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOArrayMath.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOArrayWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOInPlaceUnaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOUnaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOBinaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOAxisPack.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOScalarContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOVectorContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOTensorContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOAxis.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOVisibilities.hh
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#install library (headers only) ################################################
hops_install_headers (${CONTAINER_HEADERFILES})

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
