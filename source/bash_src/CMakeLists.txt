################################################################################
#configure and install the environment set-up script
configure_file( ${SHSOURCE}/hopsenv.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/hops.bash @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hops.bash DESTINATION ${BIN_INSTALL_DIR} )

#configure and install the vhelp script
configure_file( ${SHSOURCE}/vhelp.sh.in ${CMAKE_CURRENT_BINARY_DIR}/vhelp @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vhelp DESTINATION ${BIN_INSTALL_DIR} )
