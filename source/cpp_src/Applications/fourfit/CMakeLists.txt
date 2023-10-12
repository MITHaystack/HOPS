# pre-requisites ###############################################################
find_package (PGPLOT)
find_package (FFTW)
find_package (X11)
find_package (GFORTRAN)

if( PGPLOT_FOUND AND FFTW_FOUND AND X11_FOUND AND GFORTRAN_FOUND )

    #declare exe name ############################################################
    set(EXENAME "nufourfit")

    #list header files #############################################################

    file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

    set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

    foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
    	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
    	list( APPEND AM_${EXENAME}_HEADERFILES "./include/${BASENAME}" )
    endforeach( BASENAME )

    #list source files #############################################################

    file(GLOB ${EXENAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

    set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

    foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
    	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
    	list( APPEND AM_${EXENAME}_SOURCEFILES "${BASENAME}" )
    endforeach( BASENAME )

    #declare relative paths to internal headers/libs ###############################
    set(MSG_RELPATH "../../../c_src/msg")
    set(MK4UTIL_RELPATH "../../../c_src/mk4util")
    set(AFIO_RELPATH "../../../c_src/afio")
    set(DFIO_RELPATH "../../../c_src/dfio")
    set(VEX_RELPATH "../../../c_src/vex")
    set(FFCONTROL_RELPATH "../../../c_src/fourfit_libs/ffcontrol")
    set(FFCORE_RELPATH "../../../c_src/fourfit_libs/ffcore")
    set(FFMATH_RELPATH "../../../c_src/fourfit_libs/ffmath")
    set(FFIO_RELPATH "../../../c_src/fourfit_libs/ffio")
    set(FFSEARCH_RELPATH "../../../c_src/fourfit_libs/ffsearch")
    set(FFPLOT_RELPATH "../../../c_src/fourfit_libs/ffplot")

    list(APPEND RELPATH_LIST
        ${MSG_RELPATH}
        ${MK4UTIL_RELPATH}
        ${AFIO_RELPATH}
        ${DFIO_RELPATH}
        ${VEX_RELPATH}
        ${FFCONTROL_RELPATH}
        ${FFMATH_RELPATH}
        ${FFCORE_RELPATH}
        ${FFIO_RELPATH}
        ${FFSEARCH_RELPATH}
        ${FFPLOT_RELPATH}
    )

    #include internal library headers
    foreach(path ${RELPATH_LIST})
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
    endforeach(path)

    #include exe headers
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

    #special include for hops_config.h (which is generated at build time)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../../c_src/)

    #include external library headers
    include_directories(${PGPLOT_INCLUDE_DIR})
    include_directories(${FFTW_INCLUDE_DIR})

    #yes...the number of escapes '\' needed here is ridiculous,
    #but there is no other easier way to pass these flags to automake
    #(this has not been tested on platforms other than bash/linux)
    if(HOPS3_USE_CXX)
        set(FF_PROGNAME_DEF "FF_PROGNAME=\\\"fourfit\\\"")
        set(FF_VER_NO_DEF "FF_VER_NO=\\\"${MODULE_VERSION_NUMBER}\\\"")
    else()
        set(FF_PROGNAME_DEF "FF_PROGNAME=\\\\\"fourfit\\\\\"")
        set(FF_VER_NO_DEF "FF_VER_NO=\\\\\"${MODULE_VERSION_NUMBER}\\\\\"")
    endif(HOPS3_USE_CXX)

    if(HOPS3_USE_CXX)
        SET_SOURCE_FILES_PROPERTIES(${${EXENAME}_SOURCEFILES} PROPERTIES LANGUAGE CXX )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${FF_PROGNAME_DEF}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${FF_VER_NO_DEF}")
    else()
        hops_add_cflag(${FF_PROGNAME_DEF})
        hops_add_cflag(${FF_VER_NO_DEF})
    endif(HOPS3_USE_CXX)

    #collect all the other include dir
    hops_internal_include_directories("./")

    # set library lists ############################################################
    set(INTERNAL_LIBLIST
        msg
        mk4util
        afio
        dfio
        vex
        ffcontrol
        ffmath
        ffcore
        ffio
        ffsearch
        ffplot
    )

    #add the new C++ libraries, so we can use them too
    list(APPEND INTERNAL_LIBLIST
        MHO_Message
        MHO_Utilities
        MHO_Vex2JSON
        MHO_Math
        MHO_Containers
        MHO_Operators
        MHO_Calibration
    )

    set(EXTERNAL_LIBLIST
        ${FFTW_LIBRARIES}
        ${PGPLOT_LIBRARIES}
        ${GFORTRAN_LIBRARIES}
        ${X11_LIBRARIES}
        "m"
    )

    add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
    target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

    hops_install_headers (${${EXENAME}_HEADERFILES})
    hops_install_executables(${EXENAME})

endif()
