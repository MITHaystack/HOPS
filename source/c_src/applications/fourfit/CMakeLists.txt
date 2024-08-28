# pre-requisites ###############################################################
find_package (PGPLOT)
find_package (FFTW)
find_package (X11)
find_package (GFORTRAN)

if( PGPLOT_FOUND AND FFTW_FOUND AND X11_FOUND AND GFORTRAN_FOUND )

    #declare exe name ############################################################
    set(EXENAME "fourfit3")

    #list header files #############################################################

    file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

    set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

    foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
    	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
    endforeach( BASENAME )

    #list source files #############################################################

    file(GLOB ${EXENAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

    set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

    foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
    	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
    endforeach( BASENAME )

    #declare relative paths to internal headers/libs ###############################
    set(MSG_RELPATH "../../msg")
    set(MK4UTIL_RELPATH "../../mk4util")
    set(AFIO_RELPATH "../../afio")
    set(DFIO_RELPATH "../../dfio")
    set(VEX_RELPATH "../../vex")
    set(FFCONTROL_RELPATH "../../fourfit_libs/ffcontrol")
    set(FFCORE_RELPATH "../../fourfit_libs/ffcore")
    set(FFMATH_RELPATH "../../fourfit_libs/ffmath")
    set(FFIO_RELPATH "../../fourfit_libs/ffio")
    set(FFSEARCH_RELPATH "../../fourfit_libs/ffsearch")
    set(FFPLOT_RELPATH "../../fourfit_libs/ffplot")

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

    # set library lists ############################################################
    set(INTERNAL_LIBLIST
        "msg"
        "mk4util"
        "afio"
        "dfio"
        "vex"
        "ffcontrol"
        "ffmath"
        "ffcore"
        "ffio"
        "ffsearch"
        "ffplot"
    )

    set(EXTERNAL_LIBLIST
        ${FFTW_LIBRARIES}
        ${X11_LIBRARIES}
        ${PGPLOT_LIBRARIES}
        ${GFORTRAN_LIBRARIES}
        ${X11_LIBRARIES}
        "m"
    )

    add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
    target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

    hops_install_headers (${${EXENAME}_HEADERFILES})
    hops_install_executables(${EXENAME})

    #fourfit is not fourfit4...so install a symlink as: fourfit -> fourfit3
    if(NOT HOPS_IS_HOPS4)
        hops_install_symlink( "${BIN_INSTALL_DIR}/${EXENAME}" "${BIN_INSTALL_DIR}/fourfit" )
    endif()

else()
    
    if( NOT PGPLOT_FOUND )
        message(WARNING "PGPLOT library was not found, will not build and install fourfit3.")
    endif()
    
    if( NOT FFTW_FOUND )
            message(WARNING "FFTW3 library was not found, will not build and install fourfit3.")
    endif()
    
    if( NOT X11_FOUND )
            message(WARNING "X11 library was not found, will not build and install fourfit3.")
    endif()
    
    if( NOT GFORTRAN_FOUND )
            message(WARNING "gfortran compiler/library was not found, will not build and install fourfit3.")
    endif()
    
endif()
