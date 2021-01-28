#list header files #############################################################
set (CONTAINER_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayMath.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_AxisPack.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ScalarContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VectorContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TableContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Axis.hh
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#install library (headers only) ################################################
hops_install_headers (${CONTAINER_HEADERFILES})

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
