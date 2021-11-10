#declare exe name ############################################################
set(EXENAME "snratio")

#list source files #############################################################
list(INSERT ${EXENAME}_SOURCEFILES 0 ${CMAKE_CURRENT_SOURCE_DIR}/snratio.c)

#declare relative paths to internal headers/libs ###############################
set(MSG_RELPATH "../../msg")
set(MK4UTIL_RELPATH "../../mk4util")
set(AFIO_RELPATH "../../afio")

list(APPEND RELPATH_LIST
    ${MSG_RELPATH}
    ${MK4UTIL_RELPATH}
    ${AFIO_RELPATH}
)


# set library lists ############################################################
set(INTERNAL_LIBLIST
    "msg"
    "mk4util"
    "afio"
)

#math library
set(EXTERNAL_LIBLIST m)

#include internal library headers
foreach(path ${RELPATH_LIST})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

hops_install_executables(${EXENAME})
