#!/bin/sh
#
# check script 3413
#
# run fourfit on the captive data 3413
# this is a broadband experiment,  Mk4/DiFX
#

# set final exit status as an ERROR in case you forget to set it
passfail=99
# setups for test $something; exit=echo to disable exits 1..4
something=3413
[ -x "$MHO_REGRESSION_DATA/switches/test_envchk.sh" ] &&
    . "$MHO_REGRESSION_DATA/switches/test_envchk.sh"

# declare the tarballs that are needed and make those arrangements
tarballs='3413'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables=''
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/3413
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -f "$data/cf3413" ] || { echo config file missing ; exit 6; }
os=`uname -s` || os=idunno
grep -v $os $data/cf3413 > ./cf3413
rm -f ff-3413.ps

# more of a unit test here
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# second execute some tests and set $passfail appropriately
$verb && echo \
$fourfit -t -d diskfile:ff-3413.ps -b GE -P LL \\ && echo \
    -c ./cf3413 $data/278-1758/0552+398.wmtukg
$fourfit -t -d diskfile:ff-3413.ps -b GE -P LL \
    -c ./cf3413 $data/278-1758/0552+398.wmtukg
[ -f ./ff-3413.ps ] || { echo ./ff-3413.ps missing && exit 7 ; }
# pluck out lines containing the snr and amp, parse and check

# ff-3413.ps:7570 9384 M (5.880) SR
line=$(grep '7570 9384' ./ff-3413.ps)
IFS='()'
read a amp b <<<"$line"
low=5.870
high=5.890
okamp=$(echo "$amp>$low && $amp<$high" | bc)
$verb && echo okamp is $okamp and "$low < $amp < $high" is expected from: $line
# ff-3413.ps:7570 9653 M (124.5) SR
lsnr=$(grep '7570 9653' ./ff-3413.ps)
IFS='()'
read a snr b <<<"$lsnr"
low=124.2
high=124.7
oksnr=$(echo "$snr>$low && $snr<$high" | bc)
$verb && echo oksnr is $oksnr and "$low < $snr < $high" is expected from: $lsnr
#
[ "$okamp" -gt 0 -a "$oksnr" -gt 0 ] && passfail=0 || passfail=8

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
