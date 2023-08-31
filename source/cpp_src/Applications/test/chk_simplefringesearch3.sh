#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/3764; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
D2H_EXP_NUM=hops4
D2M4_EXP_NUM=.
SCAN_DIR=104-1228
cd $EXP_DIR

echo "Running: ffit -d ./${D2H_EXP_NUM}/${SCAN_DIR} -c ./test0.cf -b AS -p RR"
time ffit -d ./${D2H_EXP_NUM}/${SCAN_DIR} -c ./test0.cf -b AS -p RR | tee ./sfs.out

echo "Running: fourfit -m 4 -c ./test0.cf -b AS -P RR ./${SCAN_DIR} set plot_data_dir ./chk3 "
time fourfit -m 1 -c ./test0.cf -b AS -P RR ./${SCAN_DIR} set plot_data_dir ./chk3 2>&1  | tee ./ff.out

compjsonpdd.py ./fdump.json ./chk3/104-1228-AS-B-RR.2GAB6T
RET_VAL=$?

exit $RET_VAL
