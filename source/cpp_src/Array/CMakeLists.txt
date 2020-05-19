#list header files #############################################################
set (HARRAY_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HArrayMath.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HArrayWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HInPlaceUnaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HUnaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HBinaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HScalarContainer.hh
)

#declare header paths ##########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#install library (headers only) ################################################

hops_install_headers (${HARRAY_HEADERFILES})
