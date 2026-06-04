#!/bin/sh

#initialize some vars
OLD_HOPS_INSTALL="NULL"
OLD_PATH="NULL"

#look for any old copy of the install prefix and cache it. Note that
#OLD_HOPS_INSTALL is also exported for the helper scripts (hops_buildenv.sh,
#hops_pypath.sh), which scrub the old prefix out of CMAKE_PREFIX_PATH /
#PKG_CONFIG_PATH / PYTHONPATH respectively.
if [ -n "$HOPS_INSTALL" ]
    then
        export OLD_HOPS_INSTALL="$HOPS_INSTALL"
        OLD_PATH=$OLD_HOPS_INSTALL/bin:$OLD_HOPS_INSTALL/bin/test:
fi

# Figure out the install prefix at runtime (rather than hard-coding it), so that
# the installation is relocatable. Find the path of *this* script while it is
# being sourced, then take its parent's parent: the script is installed as
# <prefix>/bin/hops.bash, so the install prefix is two levels up.
# This should work on bash/zsh but dash/sh are probably out of scope (for now)
if [ -n "${BASH_SOURCE:-}" ]; then
   # shellcheck disable=SC3028 # bash sets BASH_SOURCE
   HOPS_SCRIPT_SOURCE=${BASH_SOURCE}
elif [ -n "${ZSH_VERSION:-}" ]; then
   # shellcheck disable=all
   HOPS_SCRIPT_SOURCE=${(%):-%N}
else
   echo "ERROR: could not determine the hops install location (need bash or zsh)." >&2
   return 1
fi

#export the hops install/sys location (use path based on this script's location)
HOPS_INSTALL=$(cd "$(dirname "${HOPS_SCRIPT_SOURCE}")/.." > /dev/null 2>&1 && pwd)
export HOPS_INSTALL

#used to select the VGOS VPAL code fringe fitter (fourfit3 or fourfit4)
export HOPS_VPAL_FRINGE_FITTER=@HOPS_VPAL_FOURFIT@

#used to specify the directory where to look for user python plugins
export HOPS_DEFAULT_PLUGINS_DIR=$HOPS_INSTALL/plugin_scripts

#needed by VEX2XML
export HOPS_JAVACLASSPATH=$HOPS_INSTALL/lib

#install info
export HOPS_ARCH=@CMAKE_SYSTEM_PROCESSOR@
export HOPS_VERSION=@HOPS_VERSION_NUMBER@

#legacy env
#NOTE: PROGDOC (HOPS3 program docs) and AHELP (aedit help) are no longer set here and are deprecated.
export DEF_CONTROL=/dev/null

#replace old (system) variable instances with new values
NEW_PATH=$(printf '%s\n' "$PATH" | sed "s|$OLD_PATH||g")
NEW_PATH="$HOPS_INSTALL/bin:$HOPS_INSTALL/bin/test:$NEW_PATH"
export PATH="$NEW_PATH"

#NOTE: (1) We deliberately do NOT set LD_LIBRARY_PATH. The installed binaries and
#libraries (including the pybind11 modules)
#carry $ORIGIN-relative RUNPATHs (see cmake/HopsReproducible.cmake), so the loader
#can find the HOPS libraries relative to each binary's own location.

#NOTE: (2) CMAKE_PREFIX_PATH and PKG_CONFIG_PATH are NOT set here. They are only
#needed when *compiling/linking* downstream code against an installed HOPS, not
#for running the installed tools. To set them up, source the build-env helper:
#    . "$HOPS_INSTALL/bin/hops_buildenv.sh"

#NOTE: (3) To setup PYTHONPATH (append the HOPS site-packages dir), so that you may import
# hops related python modules into your environment use the helper script:
#     . "$HOPS_INSTALL/bin/hops_pypath.sh"
# the installed HOPS CLI (python) tools do not depend on
#this, they self-locate their modules, this is just for interactive use.

#NOTE: (3) We will also look for plugin scripts in the environmental variable:
#HOPS_USER_PLUGINS_DIR
#but it is not set here (the user should set it independently)

echo "HOPS install directory set to ${HOPS_INSTALL}"

#clean up temporary variables used during environment set-up
unset HOPS_SCRIPT_SOURCE

return 0
