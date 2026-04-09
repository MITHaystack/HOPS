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
HOPS4_DIR=105-1800b
OUT_DIR=out_test
cd $EXP_DIR
export HOPS_PLOT_DATA_MASK=0x83FFFFFF

if [ ! -d "./${D2H_EXP_NUM}" ]; then
    echo "difx2hops not run, using mark42hops converted data (105-1800a) for test"
    HOPS4_DIR="105-1800b"
fi

# echo "Running: fourfit4 -c ./cf_test5 -b GE -P I ./${D2H_EXP_NUM}/${SCAN_DIR}"
# time fourfit4 -c ./cf_test5 -b GE -P I ./${D2H_EXP_NUM}/${SCAN_DIR} | grep max555 | tee ./sfs.out

echo "Running: fourfit4 -m 4 -c ./cf_test5 -b GE -P I -o ./${D2H_EXP_NUM}/${OUT_DIR}/ ./${D2H_EXP_NUM}/${HOPS4_DIR}/"
output_file=$(fourfit4 -m 4 -c ./cf_test5 -b GE -P I -o ./${D2H_EXP_NUM}/${OUT_DIR}/ ./${D2H_EXP_NUM}/${HOPS4_DIR}/ 2>&1 | awk '{print $NF}')
echo "fourfit4 output file: $output_file"

#check if the output file was written
[ -f "./${D2H_EXP_NUM}/${OUT_DIR}/${HOPS4_DIR}/$(basename "$output_file")" ] \
  && echo "output written in OUT_DIR" \
  || { echo "output missing or wrong directory"; RET_VAL=1; }

exit $RET_VAL
