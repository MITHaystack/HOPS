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
hops_add_cxxflag(HAVE_CONFIG_H)

#fourfit displays the fringe plot with ghostscript and the postscript copypage command
#this command was disabled on recent systems so some tweaks have to changes
#need to happen in order to figure out how to display the fringe plot properly 

find_program(GS_EXE gs) 
if(GS_EXE)
    set (GS_VER_ARG "--version")
    message(STATUS "Executing command: ${GS_EXE} ${GS_VER_ARG}")
    execute_process(COMMAND ${GS_EXE} ${GS_VER_ARG} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE GS_RESULT OUTPUT_VARIABLE GS_VERSION_STRING)
    string(STRIP "${GS_VERSION_STRING}" GS_VERSION_STRING)
    message(STATUS "Ghostscript version[${GS_RESULT}]: ${GS_VERSION_STRING}")

    string(REPLACE "\n" "" GS_VERSION_VALUE ${GS_VERSION_STRING}) #strip out any errant newline chars

    #default value
    set(GS_VERSION_FLAG "0")

    if( GS_VERSION_VALUE VERSION_GREATER_EQUAL "9.1")
        set(GS_VERSION_FLAG "9100")
    endif()

    if( GS_VERSION_VALUE VERSION_GREATER_EQUAL "9.22")
        set(GS_VERSION_FLAG "9220")
    endif()

    if( GS_VERSION_VALUE VERSION_GREATER_EQUAL "9.5")
        set(GS_VERSION_FLAG "9500")
    endif()

    message(STATUS "Ghostscript version flag passed as: ${GS_VERSION_FLAG}")

    if(HOPS3_USE_CXX)
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGS_VERSION=${GS_VERSION_FLAG} ")
    else()
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGS_VERSION=${GS_VERSION_FLAG} ")
    endif(HOPS3_USE_CXX)
else()
    message(STATUS "Ghostscript executable (gs) could not be found, fourfit may not display properly.")
endif()

#old HOPS is littered with implicit function declarations
#since fixing properly this would require a header for each function 
#for the time being we are just going to disable this warning with:
#    -Wno-implicit-function-declaration and -Wno-implicit-int

#There are also a huge number of string formatting/truncation/overflow errors 
#due to all the snprintf/sprintf use that need to be fixed, so diable format with:
# -Wno-format 

option( HOPS3_DISABLE_WARNINGS "Disable specific compiler warnings in HOPS3 c-code." ON)

if(HOPS3_USE_CXX)
    if(HOPS3_DISABLE_WARNINGS)
        # Do we also need -Wno-deprecated-register ??
        set( HOPS3_CXX_WARNINGS_TO_REMOVE "-Wno-write-strings -Wno-format-security -Wno-format")
        message(STATUS "HOPS3 C++ compiler warnings disabled for: ${HOPS3_CXX_WARNINGS_TO_REMOVE}" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HOPS3_CXX_WARNINGS_TO_REMOVE}")
    endif(HOPS3_DISABLE_WARNINGS)
else()
    if(HOPS3_DISABLE_WARNINGS)
        set( HOPS3_C_WARNINGS_TO_REMOVE "-Wno-implicit-int -Wno-implicit-function-declaration -Wno-format")
        message(STATUS "HOPS3 C compiler warnings disabled for: ${HOPS3_C_WARNINGS_TO_REMOVE}" )
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${HOPS3_C_WARNINGS_TO_REMOVE}")
    endif(HOPS3_DISABLE_WARNINGS)
endif(HOPS3_USE_CXX)

option(HOPS3_USE_ADHOC_FLAGGING "Toggle adhoc flagging feature in fourfit (enabled by default)." ON)

#mk4 and vex c-libraries from old HOPS
add_subdirectory(msg)
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
add_subdirectory(applications)
