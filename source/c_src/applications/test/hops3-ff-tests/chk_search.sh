#!/bin/bash
#
# $Id: chk_search.sh 4574 2026-05-21 21:30:09Z gbc $
#
# Something to check search
#
# Note that there is search and soirch (which is better),
# so at some point the names should swap.
#

verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`pwd`
cwd=$DATADIR
$verb && echo DATADIR=$DATADIR && echo cwd=$cwd

rm -f search.ps search.out
rm -f soirch.ps soirch.out
errs=0

echo search is `type -p search`
echo soirch is `type -p soirch`
echo LD_LIBRARY_PATH=$LD_LIBRARY_PATH

# there are many -d options:
# -d all gets us the huge range ...
$verb && set -x
fringex -i 20 -d 27x27 -r alist-aedit-X0X.out |\
average -o search.avg > fas.out 2>&1

HOPS_SEARCH_REMLIMIT=0.6 \
search -g 1x1:1 -d search.ps/cps/pdf -o search.out search.avg >> fas.out 2>&1
searchrv=$?
[ "$searchrv" -eq 0 ] || errs=$(($errs+1))
echo searchrv is $searchrv errs=$errs

fringex -i 20 -d 27x27 -r alist-aedit-X0X.out |\
average -o soirch.avg > fos.out 2>&1

HOPS_SEARCH_REMLIMIT=0.6 \
soirch -d soirch.ps/cps -o soirch.out soirch.avg >> fos.out 2>&1
soirchrv=$?
[ "$soirchrv" -eq 0 ] || errs=$(($errs+1))
echo soirchrv is $soirchrv errs=$errs

$verb && set +x

ulimit -c 0
soirch -g syntax-check 2>&1
soirchsyntax=$?
# echo WE EXPECTED A SEGMENTATION FAULT HERE
[ "$soirchsyntax" -eq 1 ] || errs=$(($errs+1))
echo soirchsyntax exit return is $soirchsyntax errs=$errs

savgdifs=`diff search.avg soirch.avg | wc -l`
[ "$savgdifs" -lt 8 ] && echo search.avg soirch.avg agree || {
    errs=$(($errs+1))
    echo search.avg soirch.avg differ errs=$errs
}

set -- `ls -s search.pdf 2>&-` 0 0
size=$1
$verb && echo search size is $size
set -- `wc -l search.out` 0 0
lines=$1
$verb && echo search lines is $lines
[ -f search.pdf -a "$size" -ge 76 -a -f search.out -a "$lines" -eq 3 ] ||
    errs=$(($errs+1))
echo errs=$errs

set -- `ls -s soirch.ps 2>&-` 0 0
size=$1
$verb && echo soirch size is $size
set -- `wc -l soirch.out` 0 0
lines=$1
$verb && echo soirch lines is $lines

[ -f soirch.ps -a "$size" -ge 76 -a -f soirch.out -a "$lines" -eq 3 ] ||
    errs=$(($errs+1))
echo errs=$errs

echo exit with errs=$errs
exit $errs
#
# eof
#
