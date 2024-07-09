#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/3764; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
SCAN_DIR=104-1228
CM42H_DIR=104-1228a

cd $EXP_DIR
echo "Running: mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}"
mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}

RET_VAL=$?
exit $RET_VAL
