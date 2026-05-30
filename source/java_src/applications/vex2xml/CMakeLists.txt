################################################################################
# build VEX2XML jar and according to options the top-level script

# declare exe name
if(HOPS_VEX2XML3)
    set(EXENAME3 "VEX2XML3.sh")
    install( PROGRAMS ${EXENAME3} DESTINATION ${BIN_INSTALL_DIR} )
endif(HOPS_VEX2XML3)
if(HOPS_VEX2XML4)
    set(EXENAME4 "VEX2XML4.sh")
    install( PROGRAMS ${EXENAME4} DESTINATION ${BIN_INSTALL_DIR} )
endif(HOPS_VEX2XML4)

add_subdirectory(doc)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)
