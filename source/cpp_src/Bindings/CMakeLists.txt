find_package(pybind11 REQUIRED)

#list header files #############################################################
set (HBINDING_HEADERFILES

)

#declare header paths ##########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#install library (headers only) ################################################

hops_install_headers (${HARRAY_HEADERFILES})
