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

export HOPS_PLOT_DATA_MASK=0x83FFFFFF

echo "Running: ffit -c ./test0.cf -b AS -P RR ./${D2H_EXP_NUM}/${SCAN_DIR} "

time ffit -c ./test0.cf -b AS -P RR ./${D2H_EXP_NUM}/${SCAN_DIR} | grep max555 | tee ./sfs.out

sfs_mbd=$( cat ./sfs.out | grep -oP 'mbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
sfs_sbd=$( cat ./sfs.out | grep -oP 'sbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
sfs_dr=$(cat ./sfs.out | grep -oP 'dr [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )

#change format because bc can't handle scientific/exponent notation
sfs_mbd=$(printf '%.14f\n' $sfs_mbd)
sfs_sbd=$(printf '%.14f\n' $sfs_sbd)
sfs_dr=$(printf '%.14f\n' $sfs_dr)

echo "simple fringe mbd: $sfs_mbd"
echo "simple fringe sbd: $sfs_sbd"
echo "simple fringe dr: $sfs_dr"

echo "Running: fourfit -m 1 -t -c ./test0.cf -b AS -P RR ${EXP_DIR}/${SCAN_DIR}"
time fourfit -m 1 -t -c ./test0.cf -b AS -P RR ${EXP_DIR}/${SCAN_DIR} 2>&1  | grep max555 | tee ./ff.out

ff_mbd=$( cat ./ff.out | grep -oP 'mbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
ff_sbd=$( cat ./ff.out | grep -oP 'sbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
ff_dr=$(cat ./ff.out | grep -oP 'dr [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )

#change format because bc can't handle scientific/exponent notation
ff_mbd=$(printf '%.14f\n' $ff_mbd)
ff_sbd=$(printf '%.14f\n' $ff_sbd)
ff_dr=$(printf '%.14f\n' $ff_dr)

echo "fourfit mbd: $ff_mbd"
echo "fourfit sbd: $ff_sbd"
echo "fourfit dr: $ff_dr"

#make this test more sophisticated...relative tolerance is not a great choice
mbd_delta=$(echo "scale=14; 100.0*(($sfs_mbd - $ff_mbd)/$ff_mbd)" | bc)
dr_delta=$(echo "scale=14; 100.0*(($sfs_dr - $ff_dr)/$ff_dr)" | bc)
sbd_delta=$(echo "scale=14; 100.0*(($sfs_sbd - $ff_sbd)/$ff_sbd)" | bc)

echo "mbd % difference = $mbd_delta"
echo "sbd % difference = $sbd_delta"
echo "dr % difference = $dr_delta"

#tolerance of 0.02%
low=-0.02
high=0.02
echo "Tolerance is (+/- $high %) on mbd/sbd/dr."

aok_sbd=$(echo "$sbd_delta>$low && $sbd_delta<$high" | bc)
aok_mbd=$(echo "$mbd_delta>$low && $mbd_delta<$high" | bc)
aok_dr=$(echo "$dr_delta>$low && $dr_delta<$high" | bc)

echo "sbd aok is $aok_sbd, $sbd_delta, $low, $high"
echo "mbd aok is $aok_mbd, $mbd_delta, $low, $high"
echo "dr aok is $aok_dr, $dr_delta, $low, $high"

RET_VAL=1
if [ "$aok_mbd" -eq 1 -a "$aok_sbd" -eq 1 -a "$aok_dr" -eq 1 ]; then 
    RET_VAL=0
fi

exit $RET_VAL
