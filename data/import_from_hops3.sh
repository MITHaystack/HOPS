#!/bin/bash
#
# Front end for import of code from HOPS3 to HOPS4.
#
# This is really a developer script, HOPS4 developers should add
# to the initial case statement to configure things to make life
# easier by setting paths according to where you have SVN and GIT.
#
case $USER-`hostname` in
gbc-gebeor) HOPS_ROOT=/home/gbc/HOPS    GIT=hops-git    SNV=trunk ;;
*)          HOPS_ROOT=no-such-dir       BIT=no-git-dir  SVN=no-svn-dir ;;
esac
[ "x$HOPS_ROOT" = 'xno-such-dir' ] && { echo skipping; exit 77; }

exit 0
#
# eof
#
