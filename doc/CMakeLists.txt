
#install the fourfit.doc file so syntax error doesn't cause it to crash
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/fourfit.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/alist.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/aedit.doc )

option(BUILD_DOXYGEN_REF "Build doxygen reference documentation" OFF)
option(BUILD_LATEX_DOCS "Build latex documentation" OFF)

if(BUILD_DOXYGEN_REF)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_FILE "hops.doxygen")

        configure_file (${CMAKE_CURRENT_SOURCE_DIR}/reference/${DOXYGEN_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/reference/${DOXYGEN_FILE} @ONLY)
        set(REF_BUILD_DIR ${DOC_INSTALL_DIR}/reference)
        file(MAKE_DIRECTORY ${REF_BUILD_DIR})
        add_custom_target (reference
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/reference/${DOXYGEN_FILE}
            WORKING_DIRECTORY ${REF_BUILD_DIR}
            COMMENT "Generating API documentation with Doxygen for ${PROJECT_NAME}"
            VERBATIM
        )
        add_custom_command(TARGET reference POST_BUILD
            COMMAND ln -sf ${REF_BUILD_DIR}/html/index.html ${DOC_INSTALL_DIR}/${PROJECT_NAME}.html
            DEPENDS ${REF_BUILD_DIR}/html/index.html
            COMMENT "Updating documentation symlinks for ${PROJECT_NAME}"
        )
    else()
        message(STATUS "Doxygen must be installed in order to generate reference documentation.")
    endif(DOXYGEN_FOUND)
endif(BUILD_DOXYGEN_REF)

if(BUILD_LATEX_DOCS)
    include(UseLATEX)
    add_subdirectory(specification)
endif(BUILD_LATEX_DOCS)
