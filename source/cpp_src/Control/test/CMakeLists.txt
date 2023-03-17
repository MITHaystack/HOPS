#declare header paths for old c-libs ##########################################
set(MK4UTIL_RELPATH "../../c_src/mk4util")
set(MSG_RELPATH "../../c_src/msg")
set(FFCONTROL_RELPATH "../../c_src/fourfit_libs/ffcontrol")

list(APPEND RELPATH_LIST ${MK4UTIL_RELPATH} ${MSG_RELPATH} ${FFCONTROL_RELPATH}
)

#include library headers
foreach(path ${RELPATH_LIST})
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

set(HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Control  msg mk4util ffcontrol )


# set(SOURCE_BASENAMES
#     TestControlFileParsePCPhases
# )
# 
# foreach( BASENAME ${SOURCE_BASENAMES} )
#     add_executable( ${BASENAME} ${BASENAME}.cc )
#     target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
#     add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
# endforeach( BASENAME )

add_executable( TestControlFileParsePCPhases TestControlFileParsePCPhases.cc )
target_link_libraries( TestControlFileParsePCPhases ${HOPS_TEST_LIBS})
add_test(NAME TestControlFileParsePCPhases_test COMMAND TestControlFileParsePCPhases -c ${CMAKE_CURRENT_SOURCE_DIR}/cf_test1)


hops_install_executables( ${SOURCE_BASENAMES} )

