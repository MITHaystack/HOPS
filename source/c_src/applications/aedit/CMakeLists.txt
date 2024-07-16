# pre-requisites ###############################################################
find_package (PGPLOT)
find_package (FFTW)
find_package (X11)
find_package (GFORTRAN)

if( PGPLOT_FOUND AND FFTW_FOUND AND X11_FOUND AND GFORTRAN_FOUND )

    #declare exe name ############################################################
    set(EXENAME "aedit")

    #list header files #############################################################

    file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

    set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

    foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
    	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
    endforeach( BASENAME )

    #list source files #############################################################

    file(GLOB ${EXENAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

    set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

    foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
    	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
    endforeach( BASENAME )

    list(INSERT ${EXENAME}_SOURCEFILES 0 ${CMAKE_CURRENT_SOURCE_DIR}/aedit.c)

    #declare relative paths to internal headers/libs ###############################
    set(MSG_RELPATH "../../msg")
    set(MK4UTIL_RELPATH "../../mk4util")
    set(AFIO_RELPATH "../../afio")
    set(DFIO_RELPATH "../../dfio")
    set(VEX_RELPATH "../../vex")


    list(APPEND RELPATH_LIST
        ${MSG_RELPATH}
        ${MK4UTIL_RELPATH}
        ${AFIO_RELPATH}
        ${DFIO_RELPATH}
        ${VEX_RELPATH}
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

    # set library lists ############################################################
    set(INTERNAL_LIBLIST
        "mk4util"
        "afio"
        "dfio"
        "vex"
    )

    #important! We need to list the (static) cpgplot library twice here so
    #the linker can resolve all of the c -> fortran symbols
    set(EXTERNAL_LIBLIST
        ${FFTW_LIBRARIES}
        ${PGPLOT_LIBRARIES}
        ${X11_LIBRARIES}
        ${GFORTRAN_LIBRARIES}
        ${PGPLOT_LIBRARIES}
        m
    )

    if(HOPS3_USE_CXX)
        SET_SOURCE_FILES_PROPERTIES(${${EXENAME}_SOURCEFILES} PROPERTIES LANGUAGE CXX )
    endif(HOPS3_USE_CXX)

    add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
    target_link_libraries (${EXENAME}  ${EXTERNAL_LIBLIST} ${INTERNAL_LIBLIST} )

    hops_install_headers (${${EXENAME}_HEADERFILES})
    hops_install_executables(${EXENAME})


else()
    
    if( NOT PGPLOT_FOUND )
        message(WARNING "PGPLOT library was not found, will not build and install aedit.")
    endif()
    
    if( NOT FFTW_FOUND )
            message(WARNING "FFTW3 library was not found, will not build and install aedit.")
    endif()
    
    if( NOT X11_FOUND )
            message(WARNING "X11 library was not found, will not build and install aedit.")
    endif()
    
    if( NOT GFORTRAN_FOUND )
            message(WARNING "gfortran compiler/library was not found, will not build and install aedit.")
    endif()

endif()
