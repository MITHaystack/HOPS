#!/bin/sh
#
# Common check of environment variables and other setup to simplify
# the individual test files.  The variables srcdir, abs_top_srcdir,
# abs_top_builddir, MHO_REGRESSION_DATA may all be in the environment
# and if they are not set, no error occurs.  Obviously some of them
# need to be set correctly for this test to be useful.
#
# This script is meant to be sourced at the outset.
#

# standard verbosity setup
[ -z "$testverb" ] && testverb=0
verb=false ; [ "$testverb" -gt 0 ] && verb=true
very=false ; [ "$testverb" -gt 1 ] && very=true && verb=true
wery=false ; [ "$testverb" -gt 2 ] && wery=true && very=true && verb=true

# debug for the following logic
echo verb is $verb, very is $very, wery is $wery
$verb && {
    echo 
    echo test $something: $0
    echo srcdir: $srcdir
    echo abs_top_srcdir: $abs_top_srcdir
    echo abs_top_builddir: $abs_top_builddir
    echo data: $MHO_REGRESSION_DATA
    echo config: $MHO_REGRESSION_CONFIG
    echo extract: $MHO_REGRESSION_EXTRACT
    echo tidy: $MHO_REGRESSION_TIDY
    echo 
}

# standard checks
[ -z "$exit" ] && exit="echo exit disabled"
[ -z "$srcdir" -o -d "$srcdir" ] || {
    echo srcdir "$srcdir" not set correctly; $exit 1; }
[ -z "$abs_top_srcdir" -o -d "$abs_top_srcdir" ] || {
    echo abs_top_srcdir "$abs_top_srcdir" not set correctly; $exit 2; }
[ -z "$abs_top_builddir" -o -d "$abs_top_builddir" ] || {
    echo abs_top_builddir "$abs_top_builddir" not set correctly; $exit 3; }
[ $# -gt 0 ] && { echo $something takes no arguments; $exit 4; }
[ -z "$MHO_REGRESSION_DATA" ] && { echo MHO_REGRESSION_DATA not set; $exit 77; }
[ -x "$MHO_REGRESSION_DATA/switches/test_config.sh" ] || {
    echo "$MHO_REGRESSION_DATA/switches/test_config.sh" not found ; $exit 99; }

#
# eof
#
