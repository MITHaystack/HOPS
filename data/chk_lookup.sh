#!/bin/bash
#
# check script for lookup.sh
#
[ -z "$testverb" ] && testverb=0
verb=false ; [ -n "$testverb" ] && verb=true
very=false ; [ -n "$testverb" -a "$testverb" -gt 1 ] && very=true && verb=true
passfail=0

[ -z "$srcdir" -o -d "$srcdir" ] || {
    echo srcdir "$srcdir" not set correctly; exit 1; }
[ -z "$abs_top_builddir" -o -d "$abs_top_builddir" ] || {
    echo abs_top_builddir "$abs_top_builddir" not set correctly; exit 3; }

lookup=$srcdir/switches/lookup.sh
[ -x $lookup ] || { echo missing lookup; exit 4; }

sed -e '1,/^done...EOF/d' -e '/EOF/,$d' $lookup |\
while read exec trash
do
    [ `expr "$exec" : '#.*'` -ge 1 ] && echo skipping $exec && continue
    $verb && echo testing $exec ...
    execpath=`$lookup $exec`
    echo lookup found $exec at $execpath
    [ -x "$execpath" ] && echo executable $exec found at $execpath || {
        [ $exec = bogus ] && echo skipping bogus || {
            echo $exec not found at $execpath or not executable
            passfail=$(($passfail + 1))
        }
    }
    $verb && echo
done

exit $passfail
#
# eof
#
