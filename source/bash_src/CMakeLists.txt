################################################################################
#configure and install the environment set-up script
configure_file( ${SHSOURCE}/hopsenv.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/hops.bash @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hops.bash DESTINATION ${BIN_INSTALL_DIR} )

#configure and install the vhelp script
configure_file( ${SHSOURCE}/vhelp.sh.in ${CMAKE_CURRENT_BINARY_DIR}/vhelp @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vhelp DESTINATION ${BIN_INSTALL_DIR} )

#configure and install the git-revision script
configure_file( ${SHSOURCE}/git-revision.sh ${CMAKE_CURRENT_BINARY_DIR}/git-revision.sh @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/git-revision.sh DESTINATION ${BIN_INSTALL_DIR} )

#install the cohfit driver script (used by chk_cohfit_driver test and by users)
install( PROGRAMS ${SHSOURCE}/cohfit-driver.sh DESTINATION ${BIN_INSTALL_DIR} )

#install the pprint script (used by several applications)
install( PROGRAMS ${SHSOURCE}/pplot_print.sh DESTINATION ${BIN_INSTALL_DIR} )
