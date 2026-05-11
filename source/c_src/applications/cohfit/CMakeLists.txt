find_package(GSL)
find_package(Gnuplot)
find_package(ImageMagick COMPONENTS montage)
find_program(PS2PDF_EXECUTABLE NAMES ps2pdf)
find_package(PGPLOT)
find_package(X11)
find_package(GFORTRAN)


if(NOT GSL_FOUND)

    message(STATUS "GSL not found, disabling cohfit")

else()



    #declare exe name ############################################################
    set(EXENAME "cohfit")

    #list header files #############################################################

    file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

    set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

    foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
    	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
    endforeach( BASENAME )

    #list source files #############################################################

    file(GLOB_RECURSE ${EXENAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

    set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

    foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
    	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
    endforeach( BASENAME )

    list(INSERT ${EXENAME}_SOURCEFILES 0 ${CMAKE_CURRENT_SOURCE_DIR}/cohfit.c)

    #declare relative paths to internal headers/libs ###############################
    set(MSG_RELPATH "../../msg")
    set(MK4UTIL_RELPATH "../../mk4util")
    set(AFIO_RELPATH "../../afio")

    list(APPEND RELPATH_LIST
        ${MSG_RELPATH}
        ${MK4UTIL_RELPATH}
        ${AFIO_RELPATH}
    )

    #include internal library headers
    foreach(path ${RELPATH_LIST})
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
    endforeach(path)

    #include exe headers
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

    # set library lists ############################################################
    set(INTERNAL_LIBLIST
        "msg"
        "mk4util"
        "afio"
    )

    #math library
    set(EXTERNAL_LIBLIST m)

    #gsl libraries
    list(APPEND EXTERNAL_LIBLIST ${GSL_LIBRARIES})

    #pgplot libraries (cpgplot for C bindings, plus pgplot and its deps)
    if(PGPLOT_FOUND)
        find_package(PNG)
        if(PNG_FOUND)
            list(APPEND PGPLOT_LIBRARIES ${PNG_LIBRARIES})
        endif()
        include_directories(${PGPLOT_INCLUDE_DIR})
        list(APPEND EXTERNAL_LIBLIST
            ${PGPLOT_LIBRARIES}
            ${X11_LIBRARIES}
            ${GFORTRAN_LIBRARIES}
            ${PGPLOT_LIBRARIES}
        )
    else()
        message(WARNING "PGPLOT not found; cohfit will build but plotting will be unavailable.")
    endif()

    #add cflag specifying gnuplot executable path
    if(Gnuplot_FOUND)
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGNUPLOT=\\\"${GNUPLOT_EXECUTABLE}\\\"")
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGNUPLOT=\\\"${GNUPLOT_EXECUTABLE}\\\"")
    else()
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGNUPLOT=\\\"/bin/false\\\"")
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGNUPLOT=\\\"/bin/false\\\"")
    endif(Gnuplot_FOUND)

    #add cflag specifying ps2pdf executable path
    if(PS2PDF_EXECUTABLE)
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPS2PDF=\\\"${PS2PDF_EXECUTABLE}\\\"")
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPS2PDF=\\\"${PS2PDF_EXECUTABLE}\\\"")
    else()
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPS2PDF=\\\"/bin/false\\\"")
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPS2PDF=\\\"/bin/false\\\"")
    endif()

    #add cflag specifying montage (ImageMagick) executable path
    if(ImageMagick_montage_FOUND)
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMONTAGE=\\\"${ImageMagick_montage_EXECUTABLE}\\\"")
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMONTAGE=\\\"${ImageMagick_montage_EXECUTABLE}\\\"")
    else()
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMONTAGE=\\\"/bin/false\\\"")
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMONTAGE=\\\"/bin/false\\\"")
    endif(ImageMagick_montage_FOUND)

    if(HOPS3_USE_CXX)
        SET_SOURCE_FILES_PROPERTIES(${${EXENAME}_SOURCEFILES} PROPERTIES LANGUAGE CXX )
    endif(HOPS3_USE_CXX)

    add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
    target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

    hops_install_headers (${${EXENAME}_HEADERFILES})
    hops_install_executables(${EXENAME})


endif()
