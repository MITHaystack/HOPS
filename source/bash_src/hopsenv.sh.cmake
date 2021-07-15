#!/bin/sh

if [ -n "$HOPS_SYS" ]
    then
        export OLD_HOPS_SYS=$HOPS_SYS
        OLD_PATH=$OLD_HOPS_SYS/bin:
        OLD_LDLIBPATH=$OLD_HOPS_SYS/lib:
        OLD_CMAKE_PREF=$OLD_HOPS_SYS:
fi

if [ -n "$HOPS_SYSPY" ]
    then
        export OLD_HOPS_SYSPY=$HOPS_SYSPY
fi

if [ -n "$HOPS_INSTALL" ]
    then
        export OLD_HOPS_INSTALL=$HOPS_INSTALL
        OLD_PATH=$OLD_HOPS_INSTALL/bin:
        OLD_LDLIBPATH=$OLD_HOPS_INSTALL/lib:
        OLD_CMAKE_PREF=$OLD_HOPS_INSTALL:
fi

export HOPS_INSTALL=@CMAKE_INSTALL_PREFIX@

if [ $# -eq 0 ]
  then
    HOPS_SYS=$HOPS_INSTALL
  else
    HOPS_SYS=`readlink -f $1`
fi

export HOPS_SYS

export HOPSSYSPY=@CMAKE_INSTALL_PREFIX@/@PYTHON_SITE_PREFIX@

export PATH=$HOPS_INSTALL/bin:${PATH//${OLD_PATH}/}
export LD_LIBRARY_PATH=$HOPS_INSTALL/lib:${LD_LIBRARY_PATH//${OLD_LDLIBPATH}/}
export CMAKE_PREFIX_PATH=$HOPS_INSTALL:${CMAKE_PREFIX_PATH//${OLD_CMAKE_PREF}/}
export PYTHONPATH=${HOPSSYSPY}:${PYTHONPATH//${OLD_HOPSSYSPY}}
export PROGDOC=@CMAKE_INSTALL_PREFIX@/share/vhelp

echo -e "HOPS install directory set to ${HOPS_INSTALL}"

return 0
