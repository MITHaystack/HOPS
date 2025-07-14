
#install the fourfit.doc file so syntax error doesn't cause it to crash
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/fourfit.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/alist.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/aedit.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/fplot.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/adump.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/average.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/fourmer.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/fringex.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/search.doc )

hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/ff_control.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/ff_output.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/ff_search.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/ff_vsfrnge.doc )

hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/general.doc )

hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/libafio.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/libdfio.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/libutil.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/linkdata.doc )

hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/scripts.doc )
hops_install_share_vhelp( ${CMAKE_CURRENT_SOURCE_DIR}/vhelp/vhelp.doc )




option(BUILD_DOCS "Build documentation with doxygen/sphinx" OFF)
option(DEPLOY_DOCS "Configure documentation to deploy html pages on github" OFF)

if(BUILD_DOCS)

find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

    if(DOXYGEN_FOUND AND SPHINX_FOUND)

        set(REF_BUILD_DIR ${DOC_INSTALL_DIR}/reference)
        file(MAKE_DIRECTORY ${REF_BUILD_DIR})

        set(DOXY_REF_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
        file(MAKE_DIRECTORY ${DOXY_REF_BUILD_DIR})

        #set the URL base manually until we can find a work around for this sphinx/github pages quirk
        set(MIT_BASE_URL "https://github.mit.edu/pages/barrettj/hops-git")
        set(GITHUB_BASE_URL "https://github.com/pages/MITHaystack/HOPS")
        # we use REF_BUILD_DIR to direct sphinx-doxylink to the local file system, 
        # MIT_BASE_URL for github.mit.edu and GITHUB_BASE_URL for github.com (deployment)
        set(DOC_BASE_URL "${REF_BUILD_DIR}") 
        if(DEPLOY_DOCS)
            set(DOC_BASE_URL "${MIT_BASE_URL}") 
        endif(DEPLOY_DOCS)

        #first generate the doxygen C++ API reference
        set(DOXYGEN_FILE "hops.doxygen")
        configure_file (${CMAKE_CURRENT_SOURCE_DIR}/reference/${DOXYGEN_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/reference/${DOXYGEN_FILE} @ONLY)
        add_custom_target (doxygen_reference
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/reference/${DOXYGEN_FILE}
            WORKING_DIRECTORY ${DOXY_REF_BUILD_DIR}
            COMMENT "Generating API documentation with Doxygen for hops"
            VERBATIM
        )
        
        # Copy Doxygen common.tag into Sphinx _static
        add_custom_command(
            OUTPUT ${REF_BUILD_DIR}/_static/doxygen/common.tag
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${DOXY_REF_BUILD_DIR} ${REF_BUILD_DIR}/_static/doxygen
            DEPENDS doxygen_reference
            COMMENT "Copying Doxygen XML to Sphinx _static"
        )
        
        add_custom_target(copy_doxygen_tag
            DEPENDS ${REF_BUILD_DIR}/_static/doxygen/common.tag
        )

        # Copy Doxygen XML into Sphinx _static
        add_custom_command(
            OUTPUT ${REF_BUILD_DIR}/_static/doxygen/xml/index.xml
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${DOXY_REF_BUILD_DIR}/xml ${REF_BUILD_DIR}/_static/doxygen/xml
            DEPENDS doxygen_reference
            COMMENT "Copying Doxygen XML to Sphinx _static"
        )

        add_custom_target(copy_doxygen_xml
            DEPENDS ${REF_BUILD_DIR}/_static/doxygen/xml/index.xml
        )

        # Copy Doxygen html into Sphinx _static
        add_custom_command(
            OUTPUT ${REF_BUILD_DIR}/_static/doxygen/html/index.xml
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${DOXY_REF_BUILD_DIR}/html ${REF_BUILD_DIR}/_static/doxygen/html
            DEPENDS doxygen_reference
            COMMENT "Copying Doxygen HTML to Sphinx _static"
        )

        add_custom_target(copy_doxygen_html
            DEPENDS ${REF_BUILD_DIR}/_static/doxygen/html/index.xml
        )

        #careful...this file copy directive is only executed once
        #(it does not update if the .rst source files are changed, only if the build directory is removed!)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/reference/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/reference/)
        configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/reference/source/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/reference/source/conf.py @ONLY)

        add_custom_target(reference
            ${SPHINX_EXECUTABLE} -b html ${CMAKE_CURRENT_BINARY_DIR}/reference/source ${REF_BUILD_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/reference/source
            DEPENDS copy_doxygen_tag copy_doxygen_xml copy_doxygen_html
            COMMENT "Generating user documentation with Sphinx for hops"
            VERBATIM
        )

    endif(DOXYGEN_FOUND AND SPHINX_FOUND)

endif(BUILD_DOCS)
