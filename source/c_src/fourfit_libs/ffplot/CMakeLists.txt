# pre-requisites ###############################################################
find_package (PGPLOT)
find_package (FFTW)
find_package (X11)
find_package (GFORTRAN)

#only compile if the needed libraries are present
if( PGPLOT_FOUND AND FFTW_FOUND AND X11_FOUND AND GFORTRAN_FOUND )

    #declare library name ##########################################################
    set(LIBNAME "ffplot")

    #list header files #############################################################

    file(GLOB ${LIBNAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

    set( ${LIBNAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

    foreach( BASENAME ${${LIBNAME}_HEADER_BASENAMES} )
    	list( APPEND ${LIBNAME}_HEADERFILES ${${LIBNAME}_HEADER_PATH}/${BASENAME} )
    	list( APPEND AM_${LIBNAME}_HEADERFILES "./include/${BASENAME}" )
    endforeach( BASENAME )

    #list source files #############################################################

    file(GLOB ${LIBNAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

    set( ${LIBNAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)

    foreach( BASENAME ${${LIBNAME}_SOURCE_BASENAMES} )
    	list( APPEND ${LIBNAME}_SOURCEFILES ${${LIBNAME}_SOURCE_PATH}/${BASENAME} )
    	list( APPEND AM_${LIBNAME}_SOURCEFILES "./src/${BASENAME}" )
    endforeach( BASENAME )

    #declare header paths ##########################################################
    set(MSG_RELPATH "../../msg")
    set(MK4UTIL_RELPATH "../../mk4util")
    set(AFIO_RELPATH "../../afio")
    set(DFIO_RELPATH "../../dfio")
    set(VEX_RELPATH "../../vex")
    set(FFCONTROL_RELPATH "../ffcontrol")
    set(FFCORE_RELPATH "../ffcore")
    set(FFMATH_RELPATH "../ffmath")
    set(FFIO_RELPATH "../ffio")
    set(FFSEARCH_RELPATH "../ffsearch") #include but do not link!

    list(APPEND RELPATH_LIST ${MSG_RELPATH} ${MK4UTIL_RELPATH} ${AFIO_RELPATH} ${DFIO_RELPATH} ${VEX_RELPATH} ${FFCONTROL_RELPATH} ${FFCORE_RELPATH} ${FFIO_RELPATH} ${FFMATH_RELPATH} ${FFSEARCH_RELPATH})

    #include library headers
    foreach(path ${RELPATH_LIST})
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
    endforeach(path)

    #include our own headers
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

    #external includes
    include_directories(${PGPLOT_INCLUDE_DIR})
    include_directories(${FFTW_INCLUDE_DIR})

    #build/link/install library ####################################################
    list(APPEND LIBLIST "mk4util" "afio"
                        "dfio" "vex"
                        "ffcontrol" "ffcore"
                        "ffio" "ffmath"
                        ${X11_LIBRARIES} ${FFTW_LIBRARIES} ${GFORTRAN_LIBRARIES}
                        ${PGPLOT_LIBRARIES}
        )


    if(HOPS3_USE_CXX)
    SET_SOURCE_FILES_PROPERTIES(${${LIBNAME}_SOURCEFILES} PROPERTIES LANGUAGE CXX )
    endif(HOPS3_USE_CXX)

    add_library (${LIBNAME} SHARED ${${LIBNAME}_SOURCEFILES} ${${LIBNAME}_HEADERFILES})

    target_link_libraries (${LIBNAME} ${LIBLIST})

    hops_install_headers (${${LIBNAME}_HEADERFILES})
    hops_install_libraries (${LIBNAME})
    legacy_hops_install_headers (${${LIBNAME}_HEADERFILES})
    legacy_hops_install_libraries (${LIBNAME})


endif()
