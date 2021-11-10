#!/bin/bash
#
# Front end for import of code from HOPS3 to HOPS4.
#
# This is really a developer script, HOPS4 developers should add
# to the initial case statement to configure things to make life
# easier by setting paths according to where you have SVN and GIT.
#
# see bootstrap/import_scripts/import_hops.sh for help.
#
[ $# -eq 0 ] && set -- --checksum-only

case $USER-`hostname` in
gbc-gebeor) HOPS_ROOT=/home/gbc/HOPS    GIT=hops-git    SVN=trunk ;;
gbc-demi)   # two checkouts: nightly and gbc
    cwd=`pwd` ; parent=`dirname "$cwd"` ; grandparent=`dirname $parent`
    if [ "$grandparent" = '/home/gbc/HOPS' ] ; then # gbc checkout
            HOPS_ROOT=/home/gbc/HOPS    GIT=hops-git    SVN=trunk
    elif [ "$grandparent" = '/swc/HOPS4' ] ; then # gbc nightly checkout
            HOPS_ROOT=/swc/HOPS4        GIT=hops-git-am SVN=../hops/trunk
    else
        echo skipping unconfigured gbc on demi; exit 77;
    fi
    ;;
# add cases here...
*)          HOPS_ROOT=no-such-dir       GIT=no-git-dir  SVN=no-svn-dir ;;
esac
[ "x$HOPS_ROOT" = 'xno-such-dir' ] && { echo skipping; exit 77; }

# only run if current directory has normal relation to GIT
# this excludes distcheck and similar builds; normal check
# invocation places us in $HOPS_ROOT/<build>/data with our
# source in $HOPS_ROOT/$GIT/data, so:
[ -d ../../$GIT/data ] || { echo abnormal build/GIT relationship; exit 77; }

# top of the food-chain
bsi=$HOPS_ROOT/$GIT/data/bootstrap/import_scripts
HOPS4_SRC_DIR=$HOPS_ROOT/$GIT
HOPS3_SRC_DIR=$HOPS_ROOT/$SVN

# sanity check on configuration
[ -d "$HOPS4_SRC_DIR" -a -d "$HOPS3_SRC_DIR" ] || {
    echo missing HOPS4 or HOPS3 source directories
    echo HOPS4_SRC_DIR=$HOPS_ROOT/$GIT
    echo HOPS3_SRC_DIR=$HOPS_ROOT/$SVN
    exit 99
}

# standard setup follows
[ -z "$testverb" ] && testverb=0
verb=false ; [ "$testverb" -gt 0 ] && verb=true
very=false ; [ "$testverb" -gt 1 ] && very=true && verb=true
wery=false ; [ "$testverb" -gt 2 ] && wery=true && very=true && verb=true

# note that all args are passed, and the first arg
# must be one of the ones that import_hops.sh accepts.
source $bsi/import_hops.sh $@
errors=$?
echo
[ "$errors" -gt 1 ] && echo Found $errors issues or config error \#$errors
[ "$errors" -eq 1 ] && echo Found $errors issue or config error \#$errors
[ "$errors" -eq 0 ] && echo Found no issues
echo
exit $errors
#
# eof
#
