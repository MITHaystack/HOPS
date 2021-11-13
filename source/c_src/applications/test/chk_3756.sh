#!/bin/sh
#
# check script 3756
#
# run fourfit on the captive data 3756
# this is a geodetic experiment with one baseline (TEC)
#

# standard setup follows; comment out what is not needed
[ -z "$testverb" ] && testverb=0
verb=false ; [ "$testverb" -gt 0 ] && verb=true
very=false ; [ "$testverb" -gt 1 ] && very=true && verb=true
passfail=99
something=3756
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
tarballs='3756'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables=''
set -x
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }
set +x

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/3756
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -f "$data/cf_3758_GEHIMSTVY_pstokes4" ] || {
    echo config file missing ; exit 6; }

# since we rely on this for our test, make sure it is generated
rm -f ff-3756*.ps

# FIXME: these lines should go away eventually
export DEF_CONTROL=/dev/null
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# second execute some tests and set $passfail appropriately
$verb && echo \
$fourfit -t -d diskfile:ff-3756-%d.ps -b HT \\ && echo \
    -c $data/cf_3758_GEHIMSTVY_pstokes4 \\ && echo \
    $data/328-1800/1803+784.1TDGBD
$fourfit -t -d diskfile:ff-3756-%d.ps -b HT \
    -c $data/cf_3758_GEHIMSTVY_pstokes4 \
    $data/328-1800/1803+784.1TDGBD 1>&2

# there should be 8 files, and we explicitly count them below
ls ff-3756-?.ps

# pluck out lines containing the snr and amp, parse and check
filecnt=0
aoksum=0
for file in ff-3756-?.ps
do
    $verb && echo considering snr $file
    case $file in
    # ff-3756-0.ps:7570 9653 M (116.1) SR
    ff-3756-0.ps) low=116.0 high=116.2 ;;
    # ff-3756-1.ps:7570 9653 M (100.8) SR
    ff-3756-1.ps) low=100.7 high=100.9 ;;
    # ff-3756-2.ps:7570 9653 M (24.8) SR
    ff-3756-2.ps) low=24.7  high=24.9  ;;
    # ff-3756-3.ps:7570 9653 M (35.3) SR
    ff-3756-3.ps) low=35.2  high=35.4  ;;
    *) echo file is $file ;  exit 99   ;;
    esac
    line=$(grep '7570 9653' $file)
    IFS='()'
    read a snr b <<<"$line"
    IFS=''
    aok=$(echo "$snr>$low && $snr<$high" | bc)
    $verb && echo $file aok is $aok and \
        "$low < $snr < $high" is expected from: $line
    filecnt=$(($filecnt + 1))
    aoksum=$(($aoksum + $aok))
    $verb && echo filecnt: $filecnt
done
for file in ff-3756-?.ps
do
    $verb && echo considering amp $file
    case $file in
    # ff-3756-0.ps:7570 9384 M (5.535) SR
    ff-3756-0.ps) low=5.530 high=5.540 ;;
    # ff-3756-1.ps:7570 9384 M (4.805) SR
    ff-3756-1.ps) low=4.800 high=4.810 ;;
    # ff-3756-2.ps:7570 9384 M (1.182) SR
    ff-3756-2.ps) low=1.180 high=1.184 ;;
    # ff-3756-3.ps:7570 9384 M (1.683) SR
    ff-3756-3.ps) low=1.680 high=1.686 ;;
    *) echo file is $file ;  exit 99   ;;
    esac
    line=$(grep '7570 9384' $file)
    IFS='()'
    read a amp b <<<"$line"
    IFS=''
    aok=$(echo "$amp>$low && $amp<$high" | bc)
    $verb && echo $file aok is $aok and \
        "$low < $amp < $high" is expected from: $line
    filecnt=$(($filecnt + 1))
    aoksum=$(($aoksum + $aok))
    $verb && echo filecnt: $filecnt
done

$verb && echo "$aoksum" -eq 8 -a "$filecnt" -eq 8
[ "$aoksum" -eq 8 -a "$filecnt" -eq 8 ] && passfail=0 || passfail=7

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
