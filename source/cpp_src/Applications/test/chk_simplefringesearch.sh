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

echo "Running: SimpleFringeSearch -d ./${D2H_EXP_NUM}/${SCAN_DIR} -c ./cf_test3 -b GE -p XX"

SimpleFringeSearch -d ./${D2H_EXP_NUM}/${SCAN_DIR} -c ./cf_test3 -b GE -p XX | grep max555 | tee ./sfs.out

sfs_mbd=$( cat ./sfs.out | grep -oP 'mbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
sfs_sbd=$( cat ./sfs.out | grep -oP 'sbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
sfs_dr=$(cat ./sfs.out | grep -oP 'dr [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )

echo "simple fringe mbd: $sfs_mbd"
echo "simple fringe sbd: $sfs_sbd"
echo "simple fringe dr: $sfs_dr"

fourfit -m 1 -t -c ./cf_test3 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR} 2>&1  | grep max555 | tee ./ff.out

ff_mbd=$( cat ./ff.out | grep -oP 'mbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
ff_sbd=$( cat ./ff.out | grep -oP 'sbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
ff_dr=$(cat ./ff.out | grep -oP 'dr [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )

echo "fourfit mbd: $ff_mbd"
echo "fourfit sbd: $ff_sbd"
echo "fourfit dr: $ff_dr"

delta=$(echo "$sfs_mbd - $ff_mbd" | bc)
echo "mbd delta = $delta"

low=-0.0005
high=0.0005
echo "TODO: Make this tolerance more strict!"
aok=$(echo "$delta>$low && $delta<$high" | bc)
echo "aok is $aok, $delta, $low, $high"

RET_VAL=1
if [ "$aok" -eq 1 ]; then 
    RET_VAL=0
fi

exit $RET_VAL
