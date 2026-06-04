#!/bin/bash
if [ -z "$HOPS_INSTALL" ]
then
    source @CMAKE_INSTALL_PREFIX@/bin/hops.bash
else
    echo "env defined"
fi

#Ensure the HOPS python modules/bindings (hops, pyMHO_*) are importable by the
#tests, since hops.bash no longer manipulates PYTHONPATH. That now lives in
#hops_pypath.sh, so source it here:
source "$HOPS_INSTALL/bin/hops_pypath.sh"

#make sure that "srcdir" is defined to point to where the test data was stored
export srcdir=@HOPS_TEST_DATA_DIR@
