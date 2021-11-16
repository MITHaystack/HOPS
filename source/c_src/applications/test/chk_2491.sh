#!/bin/sh
#
# check script 2491
#
# This is a very old dataset captured by kad back in '94,
# in fact it is the oldest data captured, perhaps... Last seen on
# gemini:/var/ftp/pub/hops/oldhops/hops/sample_data/2491/363-200000
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
something=2491
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
tarballs='2491'
# declare the (built) executables that might not be in your path
executables='fplot'
# finally, acquire a list of directories that may need tidying
nukables=''
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/2491
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -f "$data/ff_control.2491" ] || { echo config file missing ; exit 6; }

# FIXME: these lines should go away eventually
#export DEF_CONTROL=/dev/null
#export TEXT=$abs_top_srcdir/source/c_src/vex/text

# since we rely on this for our test, make sure it is generated
rm -f ff-2491-?-??.ps

# second execute some tests and set $passfail appropriately
$fplot -p fplot-2491-A-%02d.ps $data/363-200000/??.*jznfbg
$fplot -p fplot-2491-B-%02d.ps $data/363-200000/??.*jznfqu
$fplot -p fplot-2491-C-%02d.ps $data/363-200000/??.*jzqvrk

pscount=`ls -1 ff-2491-?-??.ps | wc -l`
[ "$pscount" -eq 46 ] && passfail=0 || passfail=7

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
