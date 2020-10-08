#!/bin/bash
#
# for the automake preparations
#
[ -f README.md ] || {
    echo this must be executed in the git source directory
    exit 1
}
nuke=${1-'false'}
halt=${2-'false'}

[ "$nuke" = help ] && cat <<\EOF
    Usage: autogen.sh [true|false] [true|false]

    the first boolean will nuke the installed (ignored) files if true
    the second boolean will exit after that if true

    by default the script (re)configures the autotools in the git repo
EOF
[ "$nuke" = help ] && exit 0

$nuke && rm -rf autom4te.cache configure \
        aclocal.m4 config.guess config.sub \
        compile depcomp install-sh ltmain.sh missing \
        autoscan.log configure.scan mho_config.h.in~ \
        m4/lt~obsolete.m4 m4/ltoptions.m4 \
        m4/ltsugar.m4 m4/ltversion.m4 m4/libtool.m4
$halt && exit 0

force='--force'
forcemissing='--force-missing'
libtoolize $force
aclocal $force -I m4
autoconf $force
autoheader $force
automake --add-missing $forcemissing

cat <<EOF
    Now continue in a build directory:

    VERSION=4.00    # update as needed

    mkdir ../ambld-$VERSION
    cd ../ambld-$VERSION
    ../configure

    make all check install

    for help with configuration use:

    ../configure --help=short
EOF
#
# eof
#
