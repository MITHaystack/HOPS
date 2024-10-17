#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/vt9105; pwd`

TestPythonVisibilityAccess -d "${DATADIR}/1111/105-1800/" -b "HV"

RET_VAL=0
exit $RET_VAL
