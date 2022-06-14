#install vex 1.5 format description files
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-1.5/block-names.json" DESTINATION "${DATA_INSTALL_DIR}/vex-1.5")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-1.5/antenna.json" DESTINATION "${DATA_INSTALL_DIR}/vex-1.5")


#install the vex 2.0 format description files

#primitive block files
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/block-names.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/antenna.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/bbc.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/site.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/exper.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/freq.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/if.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/source.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/clock.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/sched.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/bitstreams.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/datastreams.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/eop.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/sefd.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/das.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/tracks.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/scheduling_params.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/extensions.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")



#high-level block files
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/global.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/station.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vex-2.0/mode.json" DESTINATION "${DATA_INSTALL_DIR}/vex-2.0")
