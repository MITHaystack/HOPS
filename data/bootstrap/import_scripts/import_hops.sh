#!/bin/bash

USAGE="$0 --checksum-only|--check|--diff|--import|--list [targets]

This script checksums files in various directories of the HOPS4 (GIT) tree
and if checksums are different, it will import the files from HOPS3 (SVN).
HOPS4_SRC_DIR and HOPS3_SRC_DIR must be defined appropriately.  The first
argument (your intent) is mandatory.  Subsequent arguments are areas of
code to consider if you do not wish to do everything; use --list to get
the list.

Option --check is a convenient shorthand for --checksum-only, and the
--diff option also outputs lines of \"$diff <hops3> <hops4>\" where
diff may be set in the environment, but defaults to vimdiff.

If further arguments are present, only the named area will be affected.
An environment variable testverb is consulted to produce for increasing
levels of verbosity (0, 1, 2).
"

# allow this to be sourced from ../../import_from_hops3.sh or alone
part='import_hops.sh'
myself=`basename $0 2>&-` || myself=sh
[ "$myself" = $part ] && return=exit || return=return

# standard setup follows
[ -z "$testverb" ] && testverb=0
verb=false ; [ "$testverb" -gt 0 ] && verb=true
very=false ; [ "$testverb" -gt 1 ] && very=true && verb=true
wery=false ; [ "$testverb" -gt 2 ] && wery=true && very=true && verb=true

arg=''
dif=false
case x${1-'--help'} in
x--check)   arg=--checksum-only ; shift ;;
x--diff)    arg=--checksum-only ; dif=true ; shift ;;
x--checksum-only)   arg=$1 ; shift ;;
x--import)          arg=$1 ; shift ;;
x--list)            arg=$1 ; shift ;;
x--help) echo "$USAGE" ; $return 0 ;;
*)       echo "$USAGE" ; $return 0 ;;
esac
[ -z "$arg" ] && { echo Must supply an argument; $return 1; }

# diff command line prep
$dif && {
    [ -z "$diff" ] && diff=vimdiff
    dnul=`echo $diff | tr '[a-z]' ' '`
}

# check on various required things and/or try to fill in the blanks
[ -n "$HOPS4_SRC_DIR" -a -n "$HOPS3_SRC_DIR" ] ||
    { echo need both HOPS4_SRC_DIR and HOPS3_SRC_DIR defined; $return 2; }
[ -d "$HOPS4_SRC_DIR" -a -d "$HOPS3_SRC_DIR" ] ||
    { echo HOPS4_SRC_DIR or HOPS3_SRC_DIR missing; $return 3; }

[ -z "$bsi" -a -n "$HOPS_ROOT" -a -n "GIT" ] &&
    bsi=$HOPS_ROOT/$GIT/data/bootstrap/import_scripts
[ -z "$bsi" -a -n "$HOPS4_SRC_DIR" ] &&
    bsi=$HOPS4_SRC_DIR/data/bootstrap/import_scripts
[ -z "$bsi" ] && bsi=where-are-the-import-scripts
[ -d "$bsi" ] || { echo nope, missing scripts: "'$bsi'" ; $return 4; }

# this list corresponds directly to the set of import scripts
# ordered by: general libraries, application specific libraries, apps, test
targets="afio dfio msg mk4util vex
    ffcontrol ffcore ffio ffmath ffplot ffsearch
    fourfit alist adump aedit
    fftest
"
[ $# -eq 0 ] && set -- $targets

echo
[ $arg == '--import' ] &&
    echo importing into HOPS4_SRC_DIR="$HOPS4_SRC_DIR" &&
    echo from directory HOPS3_SRC_DIR="$HOPS3_SRC_DIR" ||
{   echo checksum of HOPS4_SRC_DIR="$HOPS4_SRC_DIR";
    echo against the HOPS3_SRC_DIR="$HOPS3_SRC_DIR";
    $dif && echo also printing diff ... ... command lines; }
[ $arg == '--list' ] &&
    echo Targets are these: && echo $targets | fold -sw 60 | sed 's/^/    /' &&
    $return 0;
echo
errors=0
for targ
do
    [ -x $bsi/import_$targ.sh ] || {
        echo missing or not executable $bsi/import_$targ.sh
        errors=$(($errors + 1))
        continue; }
    $very && echo \# $bsi/import_$targ.sh $arg
    source $bsi/import_$targ.sh $arg
    errs=$?
    { [ "$errs" -gt 0 ] || $verb; } && echo $targ errors: $errs
    errors=$(($errors + $errs))
done

[ "$myself" = 'import_hops.sh' ] && return=exit || return=return
$return ${errors}
#
# eof
#
