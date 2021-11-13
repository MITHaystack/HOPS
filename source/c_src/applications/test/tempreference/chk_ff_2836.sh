#!/bin/bash
#
# $Id$
#
# canonical test suite for fourfit
#

if [ -z "$HOPS_SYS" ]
then
    source @CMAKE_INSTALL_PREFIX@/bin/hopsenv.sh
else
    echo "env defined"
fi

verb=false
[ -n "$testverb" ] && verb=true

os=`uname -s` || os=idunno
grep -v $os @DATADIR@/2836/cf2836 > ./cf2836

rm -f ff-2836.ps
$verb && echo \
fourfit -t -d diskfile:ff-2836.ps -b AE:X \\ && echo \
    -c ./cf2836 \\ && echo \
    @DATADIR@/2836/scan001/2145+067.olomfh

fourfit -t -d diskfile:ff-2836.ps -b AE:X \
    -c ./cf2836 \
    @DATADIR@/2836/scan001/2145+067.olomfh 2>/dev/null 1>&2
[ -f ./ff-2836.ps ] || { echo ./ff-2836.ps missing && exit 2 ; }

# pluck out line containing the snr and parse it
line=$(grep '7570 9653' ./ff-2836.ps)

IFS='()'
read a snr b <<<"$line"

# snr bounds
low=139.9
high=140.9
aok=$(echo "$snr>$low && $snr<$high" | bc)
$verb && echo aok is $aok and "$low < $snr < $high" is expected from: $line

[ "$aok" -gt 0 ]

#
# eof
#
