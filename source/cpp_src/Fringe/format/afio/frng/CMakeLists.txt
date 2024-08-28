# #install the control format description files
file(GLOB_RECURSE FRNG_FORMAT_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "*.json")

foreach( FORMAT_FILENAME ${FRNG_FORMAT_FILES} )
    install(FILES ${FORMAT_FILENAME} DESTINATION "${DATA_INSTALL_DIR}/afio/frng/")
endforeach()
