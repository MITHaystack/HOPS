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
## Lines starting with ## are explanatory, once, here, and stripped out.
##
USAGE="Usage: $0 newscriptname"
[ $# -eq 1 -a "$1" == '--help' ] && { echo "$USAGE"; exit 0; }
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
## echo $lines ; if you edit this script this number changes
[ "$lines" -eq 37 ] && echo Now edit chk_$1.sh && exit 0 || {
    echo lines was $lines ; exit 3; }
## end of copy machine
# FIXME: provide some documentation here on what this script tests
#

# set final exit status as an ERROR in case you forget to set it
passfail=99
# setups for test $something; exit=echo to disable exits 1..4
something=something
[ -x "$MHO_REGRESSION_DATA/switches/test_envchk.sh" ] &&
    . "$MHO_REGRESSION_DATA/switches/test_envchk.sh"

## you may insert other checks at this point or later

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
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
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

# first, check that everything needed is actually present
## which is to say don't spin your wheels if the test is going to be DOA
# FIXME # whatever

# second, execute some tests and set $passfail appropriately
## this is the real test, of course
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
