#!/bin/sh
#
# Script to lookup built, but not yet installed executables.
#
# This is always called with a single argument which is the name
# of the executable to locate.  For convenience the top of the
# build directory is defined as $ABD and may be used in the
# 'here' file (between <<EOF and EOF)
#
exe=${1-'executable-not-found'}
ABD=$abs_top_builddir
while read exec epath
do
    [ "$exec" = "$exe" ] && [ -x "$epath/$exe" ] && echo $epath/$exe && exit 0
done <<EOF
    #name       #path
    true        /bin
    false       /bin
    hops.bash   $ABD/source/bash_src
    #add        #...
EOF
echo $exe
exit 1
#
# eof
#
