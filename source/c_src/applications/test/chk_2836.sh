#!/bin/sh
#
# check script 2836
#
# run fourfit on captive data 2836
# This was rjc's standard go to for testing: Mk4/hdw
#

# set final exit status as an ERROR in case you forget to set it
passfail=99
# setups for test $something; exit=echo to disable exits 1..4
something=2836
[ -x "$MHO_REGRESSION_DATA/switches/test_envchk.sh" ] && 
    . "$MHO_REGRESSION_DATA/switches/test_envchk.sh"

# declare the tarballs that are needed and make those arrangements
tarballs='2836'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables='ff-2836.ps'
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first, check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/2836
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -f "$data/cf2836" ] || { echo config file missing ; exit 6; }

# since we rely on this for our test, make sure it is generated
rm -f ff-2836.ps

# FIXME: these lines should go away eventually
export DEF_CONTROL=/dev/null
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# second, execute some tests and set $passfail appropriately
$verb && echo \
$fourfit -t -d diskfile:ff-2836.ps -b AE:X \\ && echo \
    -c $data/cf2836 $data/scan001/2145+067.olomfh
$fourfit -t -d diskfile:ff-2836.ps -b AE:X \
    -c $data/cf2836 $data/scan001/2145+067.olomfh

# output file?
[ -f ./ff-2836.ps ] || { echo ./ff-2836.ps missing && exit 7; }

# FIXME: grab amp as well

# pluck out line containing the snr and parse it
line=$(grep '7570 9653' ./ff-2836.ps)

# check SNR
IFS='()'
read a snr b <<<"$line"
low=139.9
high=140.9
aok=$(echo "$snr>$low && $snr<$high" | bc)
$verb && echo aok is $aok and "$low < $snr < $high" is expected from: $line
[ "$aok" -gt 0 ] && passfail=0 || passfail=8

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
