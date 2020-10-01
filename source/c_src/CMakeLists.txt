################################################################################
#CONFIGURE AND BUILD OLD C-LIBRARIES FROM HOPS

#config header generation to mimic behavior of autoheader generated hops_config.h
#some of these are probably not necessary
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    message(STATUS "Machine is BIG_ENDIAN")
    set(WORDS_BIGENDIAN TRUE)
else()
    message(STATUS "Machine is LITTLE_ENDIAN")
    set(WORDS_BIGENDIAN FALSE)
endif()

#mimic a bunch of checks automake does (which AFAIK are not used anywhere)
include(CheckIncludeFile)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)

check_include_file(alloca.h HAVE_ALLOCA_H)
check_include_file(arpa/inet.h HAVE_ARPA_INET_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(float.h HAVE_FLOAT_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(limits.h HAVE_LIMITS_H)

check_include_file(locale.h HAVE_LOCALE_H)
check_include_file(netdb.h HAVE_NETDB_H)
check_include_file(netinet/in.h HAVE_NETINET_IN_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(stdint.h HAVE_STDINT_H)

check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file(unistd.h HAVE_UNISTD_H)

set(HOPS_SVN_REV_VALUE 0)
set(HOPS_REVISION_VALUE ${MODULE_VERSION_NUMBER})
set(PACKAGE_VERSION ${MODULE_VERSION_NUMBER})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hops_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/hops_config.h)
hops_install_headers(${CMAKE_CURRENT_BINARY_DIR}/hops_config.h)
hops_include_directories(${CMAKE_CURRENT_BINARY_DIR})
hops_add_cflag(HAVE_CONFIG_H)

#mk4 and vex c-libraries from old HOPS
add_subdirectory(mk4stub)
add_subdirectory(mk4util)
add_subdirectory(vex)
add_subdirectory(dfio)
add_subdirectory(afio)
add_subdirectory(fourfit_libs/ffcontrol)
add_subdirectory(fourfit_libs/ffmath)
add_subdirectory(fourfit_libs/ffcore)
add_subdirectory(fourfit_libs/ffio)
add_subdirectory(fourfit_libs/ffsearch)
add_subdirectory(fourfit_libs/ffplot)
add_subdirectory(applications/fourfit)
add_subdirectory(applications/alist)
add_subdirectory(applications/aedit)
