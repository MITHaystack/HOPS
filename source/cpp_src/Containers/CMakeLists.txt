#list header files #############################################################
set (CONTAINER_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHONDArrayMath.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHONDArrayWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHONDArrayFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOAxisPack.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOScalarContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOVectorContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOTableContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOAxis.hh
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#install library (headers only) ################################################
hops_install_headers (${CONTAINER_HEADERFILES})

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
