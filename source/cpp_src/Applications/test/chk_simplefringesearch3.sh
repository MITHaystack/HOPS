#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/vt9105; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
D2H_EXP_NUM=1111
D2M4_EXP_NUM=1234
SCAN_DIR=105-1800
cd $EXP_DIR

echo "Running: SimpleFringeSearch -d ./${D2H_EXP_NUM}/${SCAN_DIR} -c ./cf_test4 -b GE -p XX"
time SimpleFringeSearch -d ./${D2H_EXP_NUM}/${SCAN_DIR} -c ./cf_test4 -b GE -p XX | grep max555 | tee ./sfs.out

echo "Running: fourfit -m 4 -c ./cf_test4 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR}"
time fourfit -m 4 -c ./cf_test4 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR} 2>&1  | tee ./ff.out

FF_OUTPUT=$(cat ./ff.out |  awk '{print $2}')

echo "$FF_OUTPUT"
RET_VAL=0

exit $RET_VAL
