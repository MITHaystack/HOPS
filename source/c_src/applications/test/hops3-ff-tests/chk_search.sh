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

# dump out a config file
rm -f search.conf
search -g search.conf

HOPS_SEARCH_REMLIMIT=0.6 \
search -g 3x1:1:1:1 \
    -d search.ps/cps/pdf -o search.out -m1 search.avg >> fas.out 2>&1
searchrv=$?
[ "$searchrv" -eq 0 ] || errs=$(($errs+1))
echo searchrv is $searchrv errs=$errs

cat > search.revised.conf <<EOF
#search configuration file -- this line is mandatory
CxR:A=1x3:1:1:1
montage=1
density=100
nukegnu=0
nukedata=0
isosamples=50
dgrid3dalgorithm=qnorm
dgrid3dalgorder=3
bsplineorder=5
palette_type=rgbformulae
palette_options=34,35,36
bgfieldname=gray40
pdfsizeinch=8.000000
labelfont=Courier
titlefont=Times-New-Roman
labelfontsize=15
titlefontscale=1.500000
snrminfrac=0.500000
peakradius=0.100000
peakcolorname=black
peaktransparency=1.000000
cntr_lowest=10
cntr_increment=15
colorbar_label=SNR starting 10 by 15
EOF
HOPS_SEARCH_REMLIMIT=0.6 \
search -g search.revised.conf \
    -d searchy.ps/cps/pdf -o searchy.out -m0 search.avg > fasy.out 2>&1
cmp search.out searchy.out || {
    echo search output differs; errs=$(($errs+1)) ; echo errs is $errs; }

# made by gnuplot and montage
splotfiles=''
gnuplot=`type -p gnuplot`
[ -x "$gnuplot" ] && splotfiles="
    search-0.data search-0.gnu search-0.pdf
    search-1.data search-1.gnu search-1.pdf
    search-2.data search-2.gnu search-2.pdf
"
montage=`type -p montage`
[ -x "$montage" ] && splotfiles="$splotfiles search-4.montage.pdf"
for s in $splotfiles
do [ -s $s ] || { echo empty $s ; errs=$(($errs+1)) ; echo errs is $errs; }
done

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
[ -f search.pdf -a "$size" -ge 60 -a -f search.out -a "$lines" -eq 3 ] ||
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
