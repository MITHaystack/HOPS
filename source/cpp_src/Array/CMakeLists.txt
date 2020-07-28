#list header files #############################################################
set (HARRAY_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkArrayMath.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkArrayWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkInPlaceUnaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkUnaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkBinaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkScalarContainer.hh
)

#declare header paths ##########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#install library (headers only) ################################################

hops_install_headers (${HARRAY_HEADERFILES})
