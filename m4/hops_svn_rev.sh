#!/bin/sh
#
# Script to update hops_config.h with HOPS_SVN_REV.
#
# For the moment, we rip these things off from config.status
# CMake could use a stripped-down version of configure.ac to
# build and hardwire this information for distribution.
#
[ -f hops_config.h ] || { echo hops_config.h is missing; exit 1; }
[ -f config.status ] || { echo config.status is missing; exit 2; }

# we have to go to config.status for these as mhops_config.h
# is the last thing created...of course.
eval PACKAGE_VERSION=`grep '^S..PACKAGE_VERSION..=.' "config.status" |\
    cut -d= -f2`
eval PACKAGE_NAME=`grep '^S..PACKAGE_NAME..=.' "config.status" |\
    cut -d= -f2`
eval PACKAGE_STRING=`grep '^S..PACKAGE_STRING..=.' "config.status" |\
    cut -d= -f2`
eval PACKAGE_URL=`grep '^S..PACKAGE_URL..=.' "config.status" |\
    cut -d= -f2`
eval PACKAGE_BUGREPORT=`grep '^S..PACKAGE_BUGREPORT..=.' "config.status" |\
    cut -d= -f2`
eval PACKAGE_TARNAME=`grep '^S..PACKAGE_TARNAME..=.' "config.status" |\
    cut -d= -f2`

# placeholder for the moment
minorversion='01'
hops_svn_rev=${PACKAGE_VERSION/./}$minorversion

# ok, now fix hops_config.h
sed -e "/^#undef HOPS_SVN_REV/a\
#define HOPS_SVN_REV $hops_svn_rev

" -e "/^#undef PACKAGE_VERSION/a\
#define PACKAGE_VERSION \"$PACKAGE_VERSION\"

" -e "/^#undef PACKAGE_NAME/a\
#define PACKAGE_NAME \"$PACKAGE_NAME\"

" -e "/^#undef PACKAGE_STRING/a\
#define PACKAGE_STRING \"$PACKAGE_STRING\"

" -e "/^#undef PACKAGE_URL/a\
#define PACKAGE_URL \"$PACKAGE_URL\"

" -e "/^#undef PACKAGE_BUGREPORT/a\
#define PACKAGE_BUGREPORT \"$PACKAGE_BUGREPORT\"

" hops_config.h >> hops_config.h.tmp
mv hops_config.h.tmp hops_config.h

#
# eof
#
