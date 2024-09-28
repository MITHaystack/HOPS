
#install the fourfit.doc file so syntax error doesn't cause it to crash
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/fourfit.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/alist.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/aedit.doc )

option(BUILD_DOXYGEN_REF "Build doxygen reference documentation" OFF)
option(BUILD_LATEX_DOCS "Build latex documentation" OFF)
option(BUILD_SPHINX_DOCS "Build sphinx generated documentation" OFF)


if(BUILD_DOXYGEN_REF)
    find_package(Doxygen REQUIRED)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_FILE "hops.doxygen")
        #first generate the doxygen C++ API reference
        configure_file (${CMAKE_CURRENT_SOURCE_DIR}/reference/${DOXYGEN_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/reference/${DOXYGEN_FILE} @ONLY)
        set(REF_BUILD_DIR ${DOC_INSTALL_DIR}/reference)
        file(MAKE_DIRECTORY ${REF_BUILD_DIR})
        set(DOXY_REF_BUILD_DIR ${REF_BUILD_DIR}/_static)
        file(MAKE_DIRECTORY ${DOXY_REF_BUILD_DIR})
        add_custom_target (api-reference
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/reference/${DOXYGEN_FILE}
            WORKING_DIRECTORY ${DOXY_REF_BUILD_DIR}
            COMMENT "Generating API documentation with Doxygen for hops"
            VERBATIM
        )
    endif(DOXYGEN_FOUND)
endif(BUILD_DOXYGEN_REF)

if( BUILD_SPHINX_DOCS)
    find_package(Sphinx REQUIRED)
    if(SPHINX_FOUND)
        #careful...this file copy directive is only executed once
        #(it does not update if the .rst source files are changed, only if the build directory is removed!)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/reference/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/reference/)
        add_custom_target(reference
            ${SPHINX_EXECUTABLE} -b html ${CMAKE_CURRENT_BINARY_DIR}/reference/source ${REF_BUILD_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/reference/source
            COMMENT "Generating user documentation with Sphinx for hops"
            VERBATIM
        )
    endif(SPHINX_FOUND)
endif(BUILD_SPHINX_DOCS)


if(BUILD_LATEX_DOCS)
    include(UseLATEX)
    add_subdirectory(specification)
endif(BUILD_LATEX_DOCS)
