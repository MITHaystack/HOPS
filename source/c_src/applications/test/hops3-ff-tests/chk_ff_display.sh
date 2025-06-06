#!/bin/bash
#
# $Id: chk_ff_display.sh 4140 2023-10-18 20:47:04Z barrettj $
#
# canonical test suite for fourfit3
#

verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/testdata; pwd`

[ -n "$DISPLAY" ] || {
    $verb && echo Skipping test--DISPLAY is undefined;
    exit 0;
}

os=`uname -s` || os=idunno
grep -v $os $DATADIR/2836/cf2836 > ./cf2836

rm -f ff-display-2836.ps
$verb && echo \
fourfit3 -pt -b AE:X \\ && echo \
    -c ./cf2836 \\ && echo \
    $DATADIR/2836/scan001/2145+067.olomfh

( echo sff-display-2836.ps ; echo q ) |
fourfit3 -pt -b AE:X \
    -c ./cf2836 \
    $DATADIR/2836/scan001/2145+067.olomfh 2>/dev/null 1>&2

# pluck out line containing the snr and parse it
[ -f ./ff-display-2836.ps ] &&
    line=$(grep '7570 9653' ./ff-display-2836.ps) ||
    line='7570 9653 M (0.0) SR'

IFS='()'
read a snr b <<<"$line"

# snr bounds
low=140.0
high=141.0
aok=$(echo "$snr>$low && $snr<$high" | bc)
$verb && echo aok is $aok and "$low < $snr < $high" is expected from: $line

[ "$aok" -gt 0 ]

#
# eof
#
