option(BUILD_DOXYGEN_REF "Build doxygen reference documentation" OFF)

find_package(Doxygen)

    # Builds in the source tree (to instead build in the binary tree, switch CMAKE_CURRENT_SOURCE_DIR to CMAKE_CURRENT_BINARY_DIR in the line that starts with WORKING_DIRECTORY.
    # This is intended for use by developers only when updating the reference documentation.
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
