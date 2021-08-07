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
[ $# -eq 0 ] && echo testing the template generator && set -- template
cat > tmpsed <<EOF
3s/something/$1/
/^something=/s/=something/=$1/
/^USAGE/,/^...end.of.copy.machine/d
/^##/d
EOF
[ -f tmpsed ] || { echo temporary sed file missing; exit 2; }
sed -f tmpsed $0 > chk_$1.sh
rm tmpsed
[ -f chk_$1.sh ] && lines=`cat chk_$1.sh | wc -l` || lines=0
[ "$lines" -eq 35 ] && echo Now edit chk_$1.sh && exit 0 || exit 3
## end of copy machine
# FIXME: provide some documentation here on what this script tests
#

# standard setup follows; comment out what is not needed
## allow at least two levels of verbosity
verb=false ; [ -n "$testverb" ] && verb=true
very=false ; [ -n "$testverb" -a "$testverb" -gt 0 ] && very=true && verb=true
passfail=0
something=something
## expect to be told the srcdir location as a sanity check if nothing else
[ -d "$srcdir" ] || { echo srcdir was not set; exit 1; }
## Test scripts are normally invoked without arguments
[ $# -gt 0 ] || { echo $something takes no arguments; exit 2; }
## if the test participates in the MHO_REGRESSION system these are needed:
## just skip the test if this is not set
[ -z "$MHO_REGRESSION_DATA" ] && { echo MHO_REGRESSION_DATA not set; exit 77; }
## if it is set, consider it an error for switches/test_config.sh to be missing
[ -x "$MHO_REGRESSION_DATA/switches/test_config.sh" ] || {
    echo "$MHO_REGRESSION_DATA/switches/test_config.sh" not found ; exit 99; }

# declare the tarballs that are needed and make those arrangements
tarballs='FIXME'
## now source the config script; it should make the request data
## (i.e. what is in some tarball) present within $MHO_REGRESSION_DATA.
##
## do not forget to added the 'something' case to the configuration
## scripts for both the data and (if relevant) the requirements.txt
##
## 'unpack' 'requirements' and 'status' are used in this script so
## do not set them prior to this point or there will be problems.
source "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }
## MHO_REGRESSION_REQ will be set to 'ok' if there is no formal
## requirement that is being tracked.  The config script may also
## exit with some error code if there are problems with the config
## or the data.  If we return from the source step, we will have data
## and are good to go--data should be found in $MHO_REGRESSION_DATA/...
##
## ok, now proceed to actually test something; setting $passfail:
## 0 for PASS, 77 SKIP, 99 ERROR and anything else is a FAIL
## for FAILures, assign error numbers consecutively--helps with debugging
## also, use:
##  $verb && echo ...    # for short comments
##  $very && echo ...    # for lower level comments

# first check that everything needed is actually present
# FIXME

# second execute some tests and set $passfail appropriately
# FIXME

## MHO_REGRESSION_NUKE is set to directories to remove if
## MHO_REGRESSION_TIDY was set to tru
for dir in "MHO_REGRESSION_NUKE" ; do rm -rf $dir ; done
## scripts should exit with 0 if all went well
[ "$MHO_REGRESSION_REQ" = ok ] || echo $something $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
