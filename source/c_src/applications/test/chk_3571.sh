#!/bin/sh
#
# check script 3571
#
# run fourfit on the captive data 3571
# another broadband experiment, Mk4/DiFX
#

# set final exit status as an ERROR in case you forget to set it
passfail=99
# setups for test $something; exit=echo to disable exits 1..4
something=3571
[ -x "$MHO_REGRESSION_DATA/switches/test_envchk.sh" ] &&
    . "$MHO_REGRESSION_DATA/switches/test_envchk.sh"

# declare the tarballs that are needed and make those arrangements
tarballs='3571'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables=''
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/3571
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -f "$data/cf3571_244-1249" ] || { echo config file missing ; exit 6; }
os=`uname -s` || os=idunno
grep -v $os $data/cf3571_244-1249 > ./cf3571

# since we rely on this for our test, make sure it is generated
rm -f ff-3571.ps

# FIXME: these lines should go away eventually
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# second execute some tests and set $passfail appropriately
$verb && echo \
$fourfit -pt -d diskfile:ff-3571.ps -b GE \\ && echo \
    -c ./cf3571 -PI $data/244-1717/0727-115.zbgwce
$fourfit -pt -d diskfile:ff-3571.ps -b GE \
    -c ./cf3571 -PI $data/244-1717/0727-115.zbgwce

# does the output file exist?
[ -f ./ff-3571.ps ] || { echo ./ff-3571.ps missing && exit 7 ; }

# FIXME: grab amp as well

# pluck out line containing the snr and parse it
line=$(grep '7570 9653' ./ff-3571.ps)
IFS='()'
read a snr b <<<"$line"
# snr bounds
low=395.9
high=401
#high=396.3 (this value is from before the change to IXY fourfit amplitudes)
aok=$(echo "$snr>$low && $snr<$high" | bc)
$verb && echo aok is $aok and "$low < $snr < $high" is expected from: $line
#
[ "$aok" -gt 0 ] && passfail=0 || passfail=8

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
