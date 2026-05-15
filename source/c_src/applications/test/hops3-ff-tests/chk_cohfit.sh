#!/bin/sh
#
# $Id: chk_cohfit.sh 4517 2026-05-11 17:23:05Z gbc $
#
# exercises cohfit
#

verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`pwd`

rdir="2843/321-1701_0552+398"
targ="0552+398"
time=oifhak

[ -d $DATADIR/$rdir ] || { echo Missing 2843 data; exit 2;}

ls -l alist-aedit.coavg
che=`type -p cohfit`
[ -x $che ] || { echo No cohfit executable ; exit 77; }
echo cohfit is $che
viable=`cohfit < /dev/null | grep Wrote`
[ "$viable" = "cohfit: Wrote 0 coherence-analyzed output records" ] || {
    echo This is a you-lose cohfit, so we punt ; exit 77; }

$verb && echo \
cohfit -d alist-aedit.ps/ps \\ && echo \
'   -o alist-aedit.cohfit alist-aedit.coavg' \\ && echo \
'   1>alist-aedit-1.cohfit 2>alist-aedit-2.cohfit'

cohfit -d alist-aedit.ps/ps \
    -o alist-aedit.cohfit alist-aedit.coavg \
    1>alist-aedit-1.cohfit 2>alist-aedit-2.cohfit
echo return status $?

set -- `wc -l alist-aedit.cohfit alist-aedit-1.cohfit alist-aedit-2.cohfit`
$verb && echo wc is $@

echo "$1" == 6 -a "$3" == 0 -a "$5" == 90
[ "$1" -eq 6 -a "$3" -eq 0 -a "$5" -eq 90 ]

#
# eof
#
