#!/bin/sh
#
[ $# -ne 2 ] && {
    echo USAGE: $0 GIT-dir Data-dir
    echo
    echo The first argument is the git source directory, the second
    echo is a target MHO_REGRESSION_DATA.  If it exists, git sources
    echo are updated with rsync.  If it does not exist, it is created.
    echo
    echo It is an error for GIT-dir Data-dir to be the same dir
    exit 1
}
[ -d $2 ] || mkdir $2
git=`cd $1 && pwd`
dat=`cd $2 && pwd`
[ "$git" = "$dat" ] && { echo two distinct directories are needed ; exit 2; }
# check that $git appears sane
[ -f $git/README.txt ] || { echo GIT does not have README.txt ; exit 3; }
[ -d $git/switches ] || { echo GIT does not have switches ; exit 4; }
[ -d $git/tarballs ] || { echo GIT does not have tarballs ; exit 5; }
[ -d $git/bootstrap ] || { echo GIT does not have bootstrap ; exit 6; }

rsync -av --exclude Makefile.am --exclude Makefile.in $1/ $2

#
# eof
#
