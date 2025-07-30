#!/bin/bash
#
# $Id: chk_ff_display.sh 4383 2025-07-20 21:38:08Z gbc $
#
# canonical test suite for fourfit3
#

verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/testdata; pwd`
echo DATADIR=$DATADIR

[ -n "$DISPLAY" ] || {
    $verb && echo Skipping test--DISPLAY is undefined;
    exit 0;
}

os=`uname -s` || os=idunno
grep -v $os $DATADIR/2836/cf2836 > ./cf2836

rm -f ff-display-2836.ps

# apparently, newer versions of GCC are so efficient that the
# commands come too soon and cause problems...no clue
[ -z "$s1" ] && s1=2
[ -z "$s2" ] && s2=2

$verb && echo -n \( sleep $s1 \; echo sff-display-2836.ps \;
$verb && echo sleep $s2 \; echo q \) \| \\
$verb && echo \
$fourfit -pt -b AE:X \\ && echo \
    -c ./cf2836 \\ && echo \
    $DATADIR/2836/scan001/2145+067.olomfh

type -f $fourfit
( sleep $s1 ; echo sff-display-2836.ps ; sleep $s2 ; echo q ) |
$fourfit -pt -b AE:X \
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
