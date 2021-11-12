#!/bin/bash
#
# $Id: chk_ff_3571.sh Thu Nov 17 10:19:27 EST 2016 jpb
#
# canonical test suite for fourfit
#

if [ -z "$HOPS_SYS" ]
then
    source @CMAKE_INSTALL_PREFIX@/bin/hops.bash
else
    echo "env defined"
fi

#make sure that "srcdir" is defined to point to where the test data was stored
export srcdir=@CMAKE_SRC_DIR@