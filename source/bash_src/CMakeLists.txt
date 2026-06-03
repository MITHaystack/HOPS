################################################################################
#configure and install the environment set-up script
configure_file( ${SHSOURCE}/hopsenv.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/hops.bash @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hops.bash DESTINATION ${BIN_INSTALL_RELDIR} )

#configure and install the PYTHONPATH helper (see hops_pypath.sh). Users should source
#this (after hops.bash) if they wish to import the HOPS python extensions into
#external python programs
configure_file( ${SHSOURCE}/hops_pypath.sh.in ${CMAKE_CURRENT_BINARY_DIR}/hops_pypath.sh @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hops_pypath.sh DESTINATION ${BIN_INSTALL_RELDIR} )

#install the build-env helper (see hops_buildenv.sh). Users should source this
#(after hops.bash) only when compiling/linking downstream code against HOPS.
#This sets up CMAKE_PREFIX_PATH / PKG_CONFIG_PATH
install( PROGRAMS ${SHSOURCE}/hops_buildenv.sh DESTINATION ${BIN_INSTALL_RELDIR} )

#configure and install the vhelp script
configure_file( ${SHSOURCE}/vhelp.sh.in ${CMAKE_CURRENT_BINARY_DIR}/vhelp @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vhelp DESTINATION ${BIN_INSTALL_RELDIR} )

#configure and install the git-revision script
configure_file( ${SHSOURCE}/git-revision.sh ${CMAKE_CURRENT_BINARY_DIR}/git-revision.sh @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/git-revision.sh DESTINATION ${BIN_INSTALL_RELDIR} )

#configure and install the source-date script (records SOURCE_DATE_EPOCH for reproducible release builds)
configure_file( ${SHSOURCE}/source-date.sh ${CMAKE_CURRENT_BINARY_DIR}/source-date.sh @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/source-date.sh DESTINATION ${BIN_INSTALL_RELDIR} )

#install the cohfit driver script (used by chk_cohfit_driver test and by users)
install( PROGRAMS ${SHSOURCE}/cohfit-driver.sh DESTINATION ${BIN_INSTALL_RELDIR} )

#install the pprint script (used by several applications)
install( PROGRAMS ${SHSOURCE}/pplot_print.sh DESTINATION ${BIN_INSTALL_RELDIR} )

#install the dependency-installer helper (apt/dnf system packages for building/running HOPS from source)
install( PROGRAMS ${SHSOURCE}/hops-install-deps.sh DESTINATION ${BIN_INSTALL_RELDIR} )
