set (json_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/json.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/json_wrapper.hh
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

##################################################

hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
