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
arg=${1-'--checksum-only'}

case $USER-`hostname` in
gbc-gebeor) HOPS_ROOT=/home/gbc/HOPS    GIT=hops-git    SVN=trunk ;;
*)          HOPS_ROOT=no-such-dir       BIT=no-git-dir  SVN=no-svn-dir ;;
esac
[ "x$HOPS_ROOT" = 'xno-such-dir' ] && { echo skipping; exit 77; }

# top of the food-chain
bsi=$HOPS_ROOT/$GIT/data/bootstrap/import_scripts
HOPS4_SRC_DIR=$HOPS_ROOT/$GIT
HOPS3_SRC_DIR=$HOPS_ROOT/$SVN

# standard setup follows
[ -z "$testverb" ] && testverb=0
verb=false ; [ "$testverb" -gt 0 ] && verb=true
very=false ; [ "$testverb" -gt 1 ] && very=true && verb=true
wery=false ; [ "$testverb" -gt 2 ] && wery=true && very=true && verb=true

# note that all args are passed, and the first
# arg needs to be --checksum-only or --import
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
