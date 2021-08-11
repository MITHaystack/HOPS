#!/bin/sh
#
# check script 2836
#
# run fourfit on captive data 2836
#

# standard setup follows; comment out what is not needed
[ -z "$testverb" ] && testverb=0
verb=false ; [ -n "$testverb" ] && verb=true
very=false ; [ -n "$testverb" -a "$testverb" -gt 1 ] && very=true && verb=true
passfail=0
something=2836
[ -z "$srcdir" -o -d "$srcdir" ] || {
    echo srcdir "$srcdir" not set correctly; exit 1; }
[ -z "$abs_top_srcdir" -o -d "$abs_top_srcdir" ] || {
    echo abs_top_srcdir "$abs_top_srcdir" not set correctly; exit 2; }
[ -z "$abs_top_builddir" -o -d "$abs_top_builddir" ] || {
    echo abs_top_builddir "$abs_top_builddir" not set correctly; exit 3; }
[ $# -gt 0 ] && { echo $something takes no arguments; exit 4; }
[ -z "$MHO_REGRESSION_DATA" ] && { echo MHO_REGRESSION_DATA not set; exit 77; }
[ -x "$MHO_REGRESSION_DATA/switches/test_config.sh" ] || {
    echo "$MHO_REGRESSION_DATA/switches/test_config.sh" not found ; exit 77; }

# declare the tarballs that are needed and make those arrangements
tarballs='2836'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables=''
[ -n "$MHO_REGRESSION_CONFIG" ] && source $MHO_REGRESSION_CONFIG ||
    source "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/2836
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -f "$data/cf2836" ] || { echo config file missing ; exit 6; }

# since we rely on this for our test, make sure it is generated
rm -f ff-2836.ps

# more of a unit test here
export DEF_CONTROL=/dev/null
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# second execute some tests and set $passfail appropriately
$verb && echo \
$fourfit -t -d diskfile:ff-2836.ps -b AE:X \\ && echo \
    -c $data/cf2836 \\ && echo \
    $data/scan001/2145+067.olomfh
$fourfit -t -d diskfile:ff-2836.ps -b AE:X \
    -c $data/cf2836 \
    $data/scan001/2145+067.olomfh

# output file?
[ -f ./ff-2836.ps ] || { echo ./ff-2836.ps missing && exit 7; }

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

for dir in $nukables ; do echo nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
