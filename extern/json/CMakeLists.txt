set (json_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann/json.hpp
)

if(HOPS_USE_PYBIND11)
    list(APPEND json_HEADERFILES ${CMAKE_CURRENT_SOURCE_DIR}/include/pybind11_json/pybind11_json.hpp)
endif(HOPS_USE_PYBIND11)

##################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
