#!/bin/sh
#
# check script something
#
## Template for a shell check script.  If provided with an argument
## it makes a copy of itself as $1.sh and with commentary stripped out.
##
## sh is sometimes, but not always bash, so use sh unless the script
## really requires bash (or other shell capabilities).  Tests may also
## be implemented in other languages, such as python or perl.
##
## Lines marked FIXME: are preserved in the new script and will need edits.
##
USAGE="Usage: $0 newscriptname"
[ $# -eq 1 -a $1 == '--help' ] && { echo "$USAGE"; exit 0; }
[ $# -eq 1 -a -f "chk_$1.sh" ] && { echo "Error: chk_$1.sh exists"; exit 1; }
cat > tmpsed <<EOF
3s/something/$1/
/^something=/s/=something/=$1/
/^USAGE/,/^...end.of.copy.machine/d
/^##/d
EOF
[ -f tmpsed ] || { echo temporary sed file missing; exit 2; }
sed -f tmpsed $0 > chk_$1.sh
rm tmpsed
chmod +x chk_$1.sh
[ -f chk_$1.sh ] && lines=`cat chk_$1.sh | wc -l` || lines=0
## echo $lines
[ "$lines" -eq 47 ] && echo Now edit chk_$1.sh && exit 0 || exit 3
## end of copy machine
# FIXME: provide some documentation here on what this script tests
#

# standard setup follows; comment out what is not needed
## allow at least two levels of verbosity
[ -z "$testverb" ] && testverb=0
verb=false ; [ "$testverb" -gt 0 ] && verb=true
very=false ; [ "$testverb" -gt 1 ] && very=true && verb=true
## after that make up appropriate variables to suit your tastes
## if you never set passfail, that will be an ERROR
passfail=99
## something is the name of the test in case you need to refer to it
something=something
## these are sometimes useful things to have
[ -z "$srcdir" -o -d "$srcdir" ] || {
    echo srcdir "$srcdir" not set correctly; exit 1; }
[ -z "$abs_top_srcdir" -o -d "$abs_top_srcdir" ] || {
    echo abs_top_srcdir "$abs_top_srcdir" not set correctly; exit 2; }
## this one is required if you lookup executables
[ -z "$abs_top_builddir" -o -d "$abs_top_builddir" ] || {
    echo abs_top_builddir "$abs_top_builddir" not set correctly; exit 3; }
## Test scripts are normally invoked without arguments
[ $# -gt 0 ] && { echo $something takes no arguments; exit 4; }
## if the test participates in the MHO_REGRESSION system these are needed:
## just skip the test if this is not set
[ -z "$MHO_REGRESSION_DATA" ] && { echo MHO_REGRESSION_DATA not set; exit 77; }
## if it is set, consider it an error for switches/test_config.sh to be missing
[ -x "$MHO_REGRESSION_DATA/switches/test_config.sh" ] || {
    echo "$MHO_REGRESSION_DATA/switches/test_config.sh" not found ; exit 99; }

## you may insert other checks at this point

# declare the tarballs that are needed and make those arrangements
tarballs='FIXME'
# declare the (built) executables that might not be in your path
executables='FIXME'
# finally, acquire a list of directories that may need tidying
## this list is augmented by MHO_REGRESSION_DATA dirs that were EXTRACTED
## but you can also use it for local cleanup of large local directories
nukables=''
## now source the config script; it should make the request data
## (i.e. what is in some tarball) present within $MHO_REGRESSION_DATA.
##
## do not forget to added the 'something' case to the configuration
## scripts for both the data and (if relevant) the requirements.txt
##
## 'unpack' 'requirements' and 'status' are used in this script so
## do not set them prior to this point or there will be problems.
[ -n "$MHO_REGRESSION_CONFIG" ] && source $MHO_REGRESSION_CONFIG ||
    source "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }
## MHO_REGRESSION_REQ will be set to 'ok' if there is no formal
## requirement that is being tracked.  The config script may also
## exit with some error code if there are problems with the config
## or the data.  If we return from the source step, we will have data
## and are good to go--data should be found in $MHO_REGRESSION_DATA/...
##
## in addition, every executable mentioned in $executables will have
## a variable created with the full path to the associated executable.
## E.g. executables='alist' should result in $alist being defined.
##
## ok, now proceed to actually test something; setting $passfail:
## 0 for PASS, 77 SKIP, 99 ERROR and anything else is a FAIL
## for FAILures, assign error numbers consecutively--helps with debugging
## also, use:
##  $verb && echo ...    # for short comments
##  $very && echo ...    # for lower level comments

# first check that everything needed is actually present
# FIXME # whatever

# second execute some tests and set $passfail appropriately
# FIXME # however

## nukables is set to directories to remove if
## MHO_REGRESSION_TIDY was set to true
eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
## scripts should exit with 0 if all went well
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
