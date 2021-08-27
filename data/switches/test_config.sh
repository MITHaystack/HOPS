#!/bin/sh
#
# Configuration file for tests
#
[ -z "$verb" ] && verb=false

# initial checking to catch structural errors
[ "x$tarballs" = xFIXME -o "x$executables" = xFIXME ] && {
    echo tarballs or executables was not configured ; exit 99;
}
[ -n "$unpack" -o -n "$requirements" -o -n "$status" ] && {
    echo one of these is set: unpack requirements status
    exit 99; }

# so that if the script something is mentioned in $requirements...
[ -z "$MHO_REGRESSION_REQUIREMENTS" ] &&
    requirements=$MHO_REGRESSION_DATA/bootstrap/requirements.txt ||
    requirements=$MHO_REGRESSION_REQUIREMENTS

# ...we can access requirements database and set MHO_REGRESSION_REQ
set -- `grep "^$something" $requirements`
[ $# -eq 0 ] && MHO_REGRESSION_REQ='ok' ||
    MHO_REGRESSION_REQ='REQUIREMENTS: '"$@"

# now to make data available

# unpack the requested data if MHO_REGRESSION_EXTRACT is unset or true
# the default for extraction is true
[ -z "$MHO_REGRESSION_EXTRACT" ] && MHO_REGRESSION_EXTRACT=true
[ "$MHO_REGRESSION_EXTRACT" = true -o "$MHO_REGRESSION_EXTRACT" = false ] ||
    { echo MHO_REGRESSION_EXTRACT must be true or false; exit 99; }

# script to actually do the unpacking work
unpack=$MHO_REGRESSION_DATA/bootstrap/legacy_unpack.sh
unpack=$MHO_REGRESSION_DATA/switches/provider.sh
[ -x "$unpack" ] || { echo unpack script $unpack is missing ; exit 99; }

# loop through the list of tarballs, unpacking and tracking what might is made
[ -n "$tarballs" ] && $MHO_REGRESSION_EXTRACT && {
    for name in $tarballs
    do
        dir=`$unpack $name`
        # this could be considered an error 99
        [ $? -eq 0 ] || { echo unable to extract $name ; exit 77; }
        $verb && echo EXTRACTed directory $dir
        nukables="$nukables $dir"
    done
}
# if MHO_REGRESSION_EXTRACT=false we want to exit 77, but defer that

# directories to nuke or not is controlled by MHO_REGRESSION_TIDY
# the default for tidiness is false
[ -z "$MHO_REGRESSION_TIDY" ] && MHO_REGRESSION_TIDY=false
[ "$MHO_REGRESSION_TIDY" = true -o "$MHO_REGRESSION_TIDY" = false ] ||
    { echo MHO_REGRESSION_TIDY must be true or false; exit 99; }
$MHO_REGRESSION_TIDY || nukables=''

# track down the lookup script
[ -z $MHO_REGRESSION_LOOKUP ] &&
    lookup=$MHO_REGRESSION_DATA/switches/lookup.sh ||
    lookup=$MHO_REGRESSION_LOOKUP
[ -x "$lookup" ] || { echo lookup script $lookup is missing ; exit 99; }
[ "$lookup" = "$MHO_REGRESSION_DATA/switches/lookup.sh" ] &&
    [ -n "$abs_top_builddir" ] ||
    { echo abs_top_builddir is not set but needed for lookups ; exit 99; }

# now track down executables: for each exe redefine it to be
# the full path to the executable, then the user may use $exe
for exe in $executables
do
    ep=`$lookup $exe`
    [ -x $ep ] || { echo executable $ep for $exe is missing ; exit 99; }
    $verb && echo LOOKUP set \$$exe is $ep
    eval $exe=$ep
done

$MHO_REGRESSION_EXTRACT || { echo Extractions disabled ; exit 77 ; }

# unset all working variables to avoid collision with caller
unset unpack requirements status ep lookup name dir exe

#
# eof
#
