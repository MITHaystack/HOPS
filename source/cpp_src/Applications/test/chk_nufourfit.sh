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

echo "Running: nufourfit -m 1 -t -c ./cf_test3 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR}"

nufourfit -m 1 -t -c ./cf_test3 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR} 2>&1  | grep max555 | tee ./nff.out

nff_mbd=$( cat ./nff.out | grep -oP 'mbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
nff_sbd=$( cat ./nff.out | grep -oP 'sbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
nff_dr=$(cat ./nff.out | grep -oP 'dr [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )

nff_mbd=$(printf '%.7f\n' $nff_mbd)
nff_sbd=$(printf '%.7f\n' $nff_sbd)
nff_dr=$(printf '%.7f\n' $nff_dr)

echo "nufourfit mbd: $nff_mbd"
echo "nufourfit sbd: $nff_sbd"
echo "nufourfit dr: $nff_dr"

fourfit -m 1 -t -c ./cf_test3 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR} 2>&1  | grep max555 | tee ./ff.out

ff_mbd=$( cat ./ff.out | grep -oP 'mbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
ff_sbd=$( cat ./ff.out | grep -oP 'sbd [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )
ff_dr=$(cat ./ff.out | grep -oP 'dr [+-]?[0-9]+([.][0-9]+)?+([e][+-][0-9]+)?' |  awk '{print $2}' )

ff_mbd=$(printf '%.7f\n' $ff_mbd)
ff_sbd=$(printf '%.7f\n' $ff_sbd)
ff_dr=$(printf '%.7f\n' $ff_dr)

echo "fourfit mbd: $ff_mbd"
echo "fourfit sbd: $ff_sbd"
echo "fourfit dr: $ff_dr"

mbd_delta=$(echo "$nff_mbd - $ff_mbd" | bc)
dr_delta=$(echo "$nff_dr - $ff_dr" | bc)
sbd_delta=$(echo "$nff_sbd - $ff_sbd" | bc)

echo "mbd delta = $mbd_delta"
echo "dr delta = $dr_delta"
echo "sbd delta = $sbd_delta"

low=-0.0000001
high=0.0000001

mbd_aok=$(echo "$mbd_delta>$low && $mbd_delta<$high" | bc)
dr_aok=$(echo "$dr_delta>$low && $dr_delta<$high" | bc)
sbd_aok=$(echo "$sbd_delta>$low && $sbd_delta<$high" | bc)

echo "mbd_aok is $mbd_aok, $mbd_delta, $low, $high"
echo "dr_aok is $dr_aok, $dr_delta, $low, $high"
echo "sbd_aok is $sbd_aok, $sbd_delta, $low, $high"


RET_VAL=1
if [ "$mbd_aok" -eq 1 ]; then 
    if [ "$dr_aok" -eq 1 ]; then 
        if [ "$sbd_aok" -eq 1 ]; then 
            RET_VAL=0
        fi
    fi
fi

exit $RET_VAL
