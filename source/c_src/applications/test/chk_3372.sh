#!/bin/sh
#
# check script 3372
#
# run fourfit on the captive data 3372
# 3372 is a scan with the legacy geodetic X band setup for comparison
# target_correlator = Haystack; 0529+483.vqbvvf  Mk4/hdw
# target_correlator = difx;     0529+483.vtqbsq  Mk4/DiFX
#

# for the moment
echo srcdir: $srcdir
echo abs_top_srcdir: $abs_top_srcdir
echo abs_top_builddir: $abs_top_builddir

# standard setup follows; comment out what is not needed
[ -z "$testverb" ] && testverb=0
verb=false ; [ "$testverb" -gt 0 ] && verb=true
very=false ; [ "$testverb" -gt 1 ] && very=true && verb=true
passfail=99
something=3372
[ -z "$srcdir" -o -d "$srcdir" ] || {
    echo srcdir "$srcdir" not set correctly; exit 1; }
[ -z "$abs_top_srcdir" -o -d "$abs_top_srcdir" ] || {
    echo abs_top_srcdir "$abs_top_srcdir" not set correctly; exit 2; }
[ -z "$abs_top_builddir" -o -d "$abs_top_builddir" ] || {
    echo abs_top_builddir "$abs_top_builddir" not set correctly; exit 3; }
[ $# -gt 0 ] && { echo $something takes no arguments; exit 4; }
[ -z "$MHO_REGRESSION_DATA" ] && { echo MHO_REGRESSION_DATA not set; exit 77; }
[ -x "$MHO_REGRESSION_DATA/switches/test_config.sh" ] || {
    echo "$MHO_REGRESSION_DATA/switches/test_config.sh" not found ; exit 99; }


# declare the tarballs that are needed and make those arrangements
tarballs='3372'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables=''
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/3372
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -f "$data/cf3372" ] || { echo config file missing ; exit 6; }
os=`uname -s` || os=idunno
grep -v $os $data/cf3372 > ./cf3372

# since we rely on this for our test, make sure it is generated
rm -f ff-3372.ps

# FIXME: these lines should go away eventually
export DEF_CONTROL=/dev/null
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# second execute some tests and set $passfail appropriately
$verb && echo \
$fourfit -t -d diskfile:ff-3372.ps -b TV:X \\ && echo \
    -c ./cf3372 $data/193-1757/0529+483.vtqbsq
$fourfit -t -d diskfile:ff-3372.ps -b TV:X \
    -c ./cf3372 $data/193-1757/0529+483.vtqbsq

# does the output file exist?
[ -f ./ff-3372.ps ] || { echo ./ff-3372.ps missing && exit 7 ; }

# pluck out lines containing the snr and amp, parse and check

# ff-3372.ps:7570 9384 M (21.078) SR
line=$(grep '7570 9384' ./ff-3372.ps)
IFS='()'
read a amp b <<<"$line"
low=21.058
high=21.088
okamp=$(echo "$amp>$low && $amp<$high" | bc)
$verb && echo okamp is $okamp and "$low < $amp < $high" is expected from: $line
# ff-2836.ps:7570 9653 M (144.1) SR
lsnr=$(grep '7570 9653' ./ff-3372.ps)
IFS='()'
read a snr b <<<"$lsnr"
low=144.0
high=144.2
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
