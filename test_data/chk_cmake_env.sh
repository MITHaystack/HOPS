#!/bin/bash
if [ -z "$HOPS_SYS" ]
then
    source @CMAKE_INSTALL_PREFIX@/bin/hops.bash
else
    echo "env defined"
fi

#make sure that "srcdir" is defined to point to where the test data was stored
export srcdir=@HOPS_TEST_DATA_DIR@
