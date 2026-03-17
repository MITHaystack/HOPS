#add julia deps
hops_internal_include_directories(../Containers/include)
hops_internal_include_directories(../Operators/include)

#list header files #############################################################
set (JLVISITOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JuliaPluginInterface.hh
)

set( JLVISITOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_JuliaPluginInterface.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Bake the julia_modules install path into interface object
# load the CxxWrap modules via @wrapmodule at runtime.
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_JuliaPluginInterface.cc
    APPEND PROPERTY COMPILE_DEFINITIONS
    JULIA_MODULES_DIR="${JULIA_MODULE_INSTALL_DIR}"
    )

#set libraries to link against ###########################
set(JLVISITOR_LIBS 
    MHO_Message 
    MHO_Utilities
    MHO_Containers 
    MHO_Operators 
    MHO_Calibration
    MHO_Initialization
    jlMHO_Containers 
    jlMHO_Operators 
    JlCxx::cxxwrap_julia 
    JlCxx::cxxwrap_julia_stl
)

#compile and install library ###################################################
add_library (MHO_jlVisitors SHARED ${JLVISITOR_SOURCEFILES})
target_link_libraries (MHO_jlVisitors ${JLVISITOR_LIBS})

hops_install_headers (${JLVISITOR_HEADERFILES})
hops_install_libraries (MHO_jlVisitors)

