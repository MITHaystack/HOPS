#!/bin/sh
#
# check script 3365
#
# This uses early HIGH/LOW 512 MHz baseband data; Mk4/hdw data
#  used by chk_fourmer, chk_frmrsrch, and chk_notches_xf
# This test does little beyond unpacking the data.
# S = SMT, O = CSO, P = SMA Phased
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
something=3365
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
tarballs='3365'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables=''
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/3365
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -d "$data/094-0644_HIGH" ] || { echo HIGH data missing ; exit 6; }
[ -d "$data/094-0644_LOW" ] || { echo LOW data missing ; exit 7; }

# since we rely on these for our test, make sure it is generated
rm -f ff-3365-*-?.ps

# FIXME: these lines should go away eventually
export DEF_CONTROL=/dev/null
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# FIXME: create man phase cals...
cat > cf3365 <<EOF
    weak_channel 0.0
    optimize_closure true
    pc_mode manual
    mbd_anchor sbd
EOF

# second execute some tests and set $passfail appropriately
# OP is CSO-SMAPhased, an easy fringe....
$verb && echo \
$fourfit -t -d diskfile:ff-3365-HIGH-%d.ps \\ && echo \
    -b OP -c cf3365 $data/094-0644_HIGH/3C273.vmudbk
$fourfit -t -d diskfile:ff-3365-HIGH-%d.ps \
    -b OP -c cf3365 $data/094-0644_HIGH/3C273.vmudbk
$verb && echo \
$fourfit -t -d diskfile:ff-3365-LOW-%d.ps \\ && echo \
    -b OP -c cf3365 $data/094-0644_LOW/3C273.vlncsf
$fourfit -t -d diskfile:ff-3365-LOW-%d.ps \
    -b OP -c cf3365 $data/094-0644_LOW/3C273.vlncsf

# there should be 2 files, and we explicitly count them below
# if we figure out how to find the other fringes we can restore
ls ff-3365-*-?.ps

hc=`ls -1 ff-3365-HIGH*ps | wc -l`
[ "$hc" -eq 1 ] || { echo missing 1 HIGH files; exit 8; }
lc=`ls -1 ff-3365-LOW*ps | wc -l`
[ "$hc" -eq 1 ] || { echo missing 1 LOW files; exit 9; }

# FIXME  with SNR/AMP tests

true && passfail=0 || passfail=10

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
