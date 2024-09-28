
#install the fourfit.doc file so syntax error doesn't cause it to crash
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/fourfit.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/alist.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/aedit.doc )

option(BUILD_DOXYGEN_REF "Build doxygen reference documentation" OFF)
option(BUILD_LATEX_DOCS "Build latex documentation" OFF)
option(BUILD_SPHINX_DOCS "Build sphinx generated documentation" OFF)

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


if(BUILD_SPHINX_DOCS)
    find_package(Sphinx REQUIRED)

    if(NOT DEFINED SPHINX_THEME)
        set(SPHINX_THEME default)
    endif()
     
    if(NOT DEFINED SPHINX_THEME_DIR)
        set(SPHINX_THEME_DIR)
    endif()
     
    # configured documentation tools and intermediate build results
    set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")
     
    # Sphinx cache with pickled ReST documents
    set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
     
    # HTML output directory
    set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")
     
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/reference/source/conf.py"
        "${BINARY_BUILD_DIR}/conf.py"
        @ONLY)
     
    add_custom_target(sphinx_docs ALL
        ${Sphinx_EXECUTABLE}
            -q -b html
            -c "${BINARY_BUILD_DIR}"
            -d "${SPHINX_CACHE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/reference/source"
            "${SPHINX_HTML_DIR}"
        COMMENT "Building HTML documentation with Sphinx")

endif(BUILD_SPHINX_DOCS)

if(BUILD_LATEX_DOCS)
    include(UseLATEX)
    add_subdirectory(specification)
endif(BUILD_LATEX_DOCS)
