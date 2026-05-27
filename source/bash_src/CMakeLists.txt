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

#configure and install the alma-vex-defs.py script
# this script is needed at correlators to support correlations with ALMA
configure_file( ${SHSOURCE}/alma-vex-defs.py.in ${CMAKE_CURRENT_BINARY_DIR}/alma-vex-defs.py @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/alma-vex-defs.py DESTINATION ${BIN_INSTALL_DIR} )

#configure and install the est_manual_phases.py script
# this script is needed in analysis to estimate manual phase cal phases
configure_file( ${SHSOURCE}/est_manual_phases.py.in ${CMAKE_CURRENT_BINARY_DIR}/est_manual_phases.py @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/est_manual_phases.py DESTINATION ${BIN_INSTALL_DIR} )

#configure and install the hops_vex2ovex.py.py script
# this script is needed to support making summary plots from aedit
configure_file( ${SHSOURCE}/hops_vex2ovex.py.py.in ${CMAKE_CURRENT_BINARY_DIR}/hops_vex2ovex.py.py @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hops_vex2ovex.py.py DESTINATION ${BIN_INSTALL_DIR} )

