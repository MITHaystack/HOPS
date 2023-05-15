#install the control format description files
#primitive block files
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/keyword-names.json" DESTINATION "${DATA_INSTALL_DIR}/control")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/dr_win.json" DESTINATION "${DATA_INSTALL_DIR}/control")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/control/freqs.json" DESTINATION "${DATA_INSTALL_DIR}/control")
