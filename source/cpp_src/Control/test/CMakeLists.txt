set(MSG_RELPATH "../../../c_src/msg/include")
set(MK4UTIL_RELPATH "../../../c_src/mk4util/include")
set(FFCONTROL_RELPATH "../../../c_src/fourfit_libs/ffcontrol/include")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MSG_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${FFCONTROL_RELPATH})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Containers/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Control msg mk4util ffcontrol)


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

