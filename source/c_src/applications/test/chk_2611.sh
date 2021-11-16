#!/bin/sh
#
# check script 2611
#
# This is the oldest data we have correlation products for...
#

# set final exit status as an ERROR in case you forget to set it
passfail=99
# setups for test $something; exit=echo to disable exits 1..4
something=2611
[ -x "$MHO_REGRESSION_DATA/switches/test_envchk.sh" ] &&
    . "$MHO_REGRESSION_DATA/switches/test_envchk.sh"

# declare the tarballs that are needed and make those arrangements
tarballs='2611'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables=''
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/2611
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }

# since we rely on this for our test, make sure it is generated
rm -f ff-2611.ps

# FIXME: these lines should go away eventually
export DEF_CONTROL=/dev/null
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# second execute some tests and set $passfail appropriately
$verb && echo \
$fourfit -t -d diskfile:ff-2611.ps -b BY \\ && echo \
    $data/062-094600/3C345.nrsiug
$fourfit -t -d diskfile:ff-2611.ps -b BY \
    $data/062-094600/3C345.nrsiug

[ -f ./ff-2611.ps ] || { echo ./ff-2611.ps missing && exit 7 ; }

# FIXME: check SNR/AMP
# FIXME: why is the plot screwed up?

true && passfail=0 || passfail=8

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
