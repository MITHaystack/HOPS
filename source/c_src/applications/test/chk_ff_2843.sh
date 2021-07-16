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

rm -f ff-2843.ps
$verb && echo \
fourfit -t -d diskfile:ff-2843.ps -b AI:S \\ && echo \
    @DATADIR@/2843/321-1701_0552+398/0552+398.oifhak \\ && echo \
    set start -3

# AIT
fourfit -t -d diskfile:ff-2843.ps -b AI:S \
    @DATADIR@/2843/321-1701_0552+398/0552+398.oifhak \
    set start -3 2>/dev/null 1>&2
[ -f ./ff-2843.ps ] || { echo ./ff-2843.ps missing && exit 2 ; }

# pluck out line containing the snr and parse it
line=$(grep '7570 9653' ./ff-2843.ps)

IFS='()'
read a snr b <<<"$line"

# snr bounds
low=47.8
high=48.6
aok=$(echo "$snr>$low && $snr<$high" | bc)
$verb && echo aok is $aok and "$low < $snr < $high" is expected from: $line

[ "$aok" -gt 0 ]
#
# eof
#
