#!/bin/bash
#
# Script to adjust KPSETEXMFPATH if the user has not set it
#
# It should either report back : or the source/doc/missing directory
#
[ -n "$KPSETEXMFPATH" ] && echo "$KPSETEXMFPATH" && exit 0
#
m4=`dirname $0`     #echo note: m4 is $m4 >&2
top=`dirname $m4`   #echo note: top is $top >&2
#
echo checking for missing texlive packages... >&2
needit=false
for x in `ls $top/doc/texmiss`
do
    xp=`kpsepath -w tex $x`
    [ -z "$xp" ] && {
        echo checking found $x is missing >&2
        needit=true
    }
done

$needit && {
    echo ":$top/doc/texmiss:" && echo checking found we need doc/texmiss >&2 ;
} || {
    echo ":" && echo checking found that nothing is missing >&2 ;
}

exit 0
#
# eof
#
