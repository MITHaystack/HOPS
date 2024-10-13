#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/3764; pwd`
export DATADIR2=`cd $srcdir/vt9105; pwd`
export DATADIR3=`cd $srcdir/3741; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
SCAN_DIR=104-1228
CM42H_DIR=104-1228a

cd $EXP_DIR
echo "Running: mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}"
mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}

EXP_DIR=$DATADIR2
SCAN_DIR=105-1800
CM42H_DIR=105-1800b
cd $EXP_DIR
mkdir -p ./1111
echo "Running: mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}"
mark42hops -i ./1234/${SCAN_DIR} -o ./1111/${CM42H_DIR}

EXP_DIR=$DATADIR3
SCAN_DIR=190-1800a
CM42H_DIR=190-1800a
cd $EXP_DIR
echo "Running: mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}"
mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}

RET_VAL=$?
exit $RET_VAL
