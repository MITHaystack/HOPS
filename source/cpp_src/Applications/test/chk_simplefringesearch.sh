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

export HOPS_PLOT_DATA_MASK=0x83FFFFFF

echo "Running: ffit -c ./cf_test4 -b GE -P XX ./${D2H_EXP_NUM}/${SCAN_DIR}"

time ffit -c ./cf_test4 -b GE -P XX ./${D2H_EXP_NUM}/${SCAN_DIR} | grep max555 | tee ./sfs.out

echo "Running: fourfit -m 1 -t -c ./cf_test4 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR}"
time fourfit -m 1 -t -c ./cf_test4 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR} set plot_data_dir ./chk1 2>&1  | grep max555 | tee ./ff.out

compjsonpdd.py ./fdump.json ./chk1/105-1800-GE-X-XX*
RET_VAL=$?

exit $RET_VAL
