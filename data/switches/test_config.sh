#!/bin/sh
#
# Configuration file for tests
#

# initial checking
[ "x$tarballs" = xFIXME ] && {
    echo tarballs was not configured ; exit 99;
}
[ -n "$unpack" -o -n "$requirements" -o -n "$status" ] && {
    echo one of these is set: unpack requirements status
    exit 99; }

# if the script something is mentioned in $requirements...
[ -z "$MHO_REGRESSION_REQUIREMENTS" ] &&
    requirements=MHO_REGRESSION_DATA/bootstrap/requirements.txt ||
    requirements=$MHO_REGRESSION_REQUIREMENTS

# ...access requirements database and set MHO_REGRESSION_REQ
set -- `grep "^$something" $requirements`
[ $# -eq 0 ] && MHO_REGRESSION_REQ='ok' || MHO_REGRESSION_REQ="$@"

# unpack the requested data if MHO_REGRESSION_EXTRACT is unset or true
[ -z "$MHO_REGRESSION_EXTRACT" ] &&
    MHO_REGRESSION_EXTRACT=true ||
    MHO_REGRESSION_EXTRACT=false
[ "$MHO_REGRESSION_EXTRACT" = true -o "$MHO_REGRESSION_EXTRACT" = false ] ||
    { echo MHO_REGRESSION_EXTRACT must be true or false; exit 99; }

# script to actually do the unpacking work
unpack=$MHO_REGRESSION_DATA/bootstrap/legacy_unpack.sh
[ -x "$unpack" ] || { echo $unpack is missing ; exit 99; }

MHO_REGRESSION_NUKE=''
[ -n "$tarballs" ] && $MHO_REGRESSION_EXTRACT && {
    for dir in $tarballs
    do
        $unpack $dir
        status=$?
        [ $status -eq 0 ] && MHO_REGRESSION_NUKE="$MHO_REGRESSION_NUKE $dir" ||
            exit $status
    done
}

# directories to nuke or not is controlled by MHO_REGRESSION_TIDY
[ -z "$MHO_REGRESSION_TIDY" ] &&
    MHO_REGRESSION_TIDY=false ||
    MHO_REGRESSION_TIDY=true
[ "$MHO_REGRESSION_TIDY" = true -o "$MHO_REGRESSION_TIDY" = false ] ||
    { echo MHO_REGRESSION_TIDY must be true or false; exit 99; }
$MHO_REGRESSION_TIDY || MHO_REGRESSION_NUKE=''

unset unpack requirements status

#
# eof
#
