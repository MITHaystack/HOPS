# #install the control format description files

file(GLOB_RECURSE FORMAT_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "*.json")

foreach( FORMAT_FILENAME ${FORMAT_FILES} )
    install(FILES ${FORMAT_FILENAME} DESTINATION "${DATA_INSTALL_DIR}/control/")
endforeach()


# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/keyword-names.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/dr_win.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/freqs.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/pc_mode.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/if.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/pc_amp_hcode.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/pc_period.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/ref_freq.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/weak_channel.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/ion_win.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/ion_npts.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/ionosphere.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/pc_phases_x.json" DESTINATION "${DATA_INSTALL_DIR}/control")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/pc_phases_y.json" DESTINATION "${DATA_INSTALL_DIR}/control")
