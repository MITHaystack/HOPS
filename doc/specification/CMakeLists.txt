option(BUILD_DOXYGEN_REF "Build doxygen reference documentation" OFF)
include(UseLATEX)

file(GLOB SPEC_INPUT_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/code/*.hh" )
message(STATUS "spec input files ${SPEC_INPUT_FILES}")

add_latex_document(
    hops-spec.tex
    INPUTS ${SPEC_INPUT_FILES}
    #BIBFILES hops-spec.bib
    #USE_BIBLATEX
)

add_custom_command(TARGET hops-spec POST_BUILD
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/hops-spec.pdf ${DOC_INSTALL_DIR}/hops-spec.pdf
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/hops-spec.pdf
    COMMENT "Installing hops-spec.pdf"
)
