#!/bin/sh

#initialize some vars
OLD_HOPS_SYS="NULL"
OLD_HOPS_SYS_PY="NULL"
OLD_HOPS_INSTALL="NULL"
OLD_CMAKE_PREF="NULL"
OLD_LD_LIBPATH="NULL"
OLD_LD_LIB64PATH="NULL"
OLD_PATH="NULL"

if [ -n "$HOPS_SYS_PY" ]
    then
        export OLD_HOPS_SYS_PY="$HOPS_SYS_PY"
fi

#look for any old copies of these variables and cache them
if [ -n "$HOPS_SYS" ]
    then
        export OLD_HOPS_SYS="$HOPS_SYS"
        OLD_PATH=$OLD_HOPS_SYS/bin:$OLD_HOPS_SYS/bin/test:
        OLD_LD_LIBPATH=$OLD_HOPS_SYS/lib:
        OLD_LD_LIB64PATH=$OLD_HOPS_SYS/lib64:
        OLD_CMAKE_PREF=$OLD_HOPS_SYS:
fi

if [ -n "$HOPS_INSTALL" ]
    then
        export OLD_HOPS_INSTALL="$HOPS_INSTALL"
        OLD_PATH=$OLD_HOPS_INSTALL/bin:$OLD_HOPS_INSTALL/bin/test:
        OLD_LD_LIBPATH=$OLD_HOPS_INSTALL/lib:
        OLD_CMAKE_PREF=$OLD_HOPS_INSTALL:
fi

export HOPS_INSTALL=@CMAKE_INSTALL_PREFIX@
#export the hops install/sys location

if [ $# -eq 0 ]
  then
    HOPS_SYS=$HOPS_INSTALL
  else
    HOPS_SYS=$(readlink -f "$1")
fi

#define some useful variables
export HOPS_SYS
export HOPS_SYS_PY=@CMAKE_INSTALL_PREFIX@/@PYTHON_SITE_PREFIX@
export HOPS_ARCH=@CMAKE_SYSTEM_PROCESSOR@
export HOPS_VERSION=@HOPS_VERSION_NUMBER@
export PROGDOC=@CMAKE_INSTALL_PREFIX@/share/vhelp
export DEF_CONTROL=/dev/null
export HOPS_VPAL_FRINGE_FITTER=@HOPS_VPAL_FOURFIT@
export HOPS_DEFAULT_PLUGINS_DIR=@PLUGINS_INSTALL_DIR@

#replace old (system) variable instances with new values
NEW_PATH=$(printf '%s\n' "$PATH" | sed "s|$OLD_PATH||g")
NEW_PATH="$HOPS_INSTALL/bin:$HOPS_INSTALL/bin/test:$NEW_PATH"
export PATH="$NEW_PATH"

NEW_LD_LIBRARY_PATH=$(printf '%s\n' "$LD_LIBRARY_PATH" | sed "s|$OLD_LD_LIBPATH||g" | sed "s|$OLD_LD_LIB64PATH||g" | sed "s|$OLD_HOPS_SYS_PY||g" )
NEW_LD_LIBRARY_PATH="$HOPS_INSTALL/lib:$HOPS_INSTALL/lib64:$HOPS_SYS_PY:$NEW_LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="$NEW_LD_LIBRARY_PATH"

NEW_CMAKE_PREFIX_PATH=$(printf '%s\n' "$CMAKE_PREFIX_PATH" | sed "s|$OLD_CMAKE_PREF||g")
NEW_CMAKE_PREFIX_PATH="$HOPS_INSTALL:$NEW_CMAKE_PREFIX_PATH"
export CMAKE_PREFIX_PATH="$NEW_CMAKE_PREFIX_PATH"

NEW_PYTHONPATH=$(printf '%s\n' "$PYTHONPATH" | sed "s|$OLD_HOPS_SYS_PY||g")
NEW_PYTHONPATH="$HOPS_SYS_PY:$NEW_PYTHONPATH"
export PYTHONPATH="$NEW_PYTHONPATH"


#NOTE: we will also look for plugin scripts in the environmental variable: HOPS_USER_PLUGINS_DIR
echo "HOPS install directory set to ${HOPS_INSTALL}"

return 0
