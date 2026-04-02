#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh


if [ ! -d "$srcdir/3764" ]; then
  echo "Directory $srcdir/3764 does not exist, will not apply this test"
  exit 127
fi

export DATADIR=`cd $srcdir/3764; pwd`
RET_VAL=0
EXP_DIR=$DATADIR
D2M4_EXP_NUM=.
MK4_SCAN_DIR=104-1228
SCAN_DIR=104-1228a
cd $EXP_DIR

export HOPS_PLOT_DATA_MASK=0x83FFFFFF

echo "Running: fourfit4 -m 4 -c ./test0.cf -b AS -P RR ./${SCAN_DIR}/"
output_file=$(fourfit4 -m 4 -c ./test0.cf -b AS -P RR ./${SCAN_DIR}/ 2>&1 | awk '{print $NF}')
echo "fourfit4 output file: $output_file"

#convert the fringe file to json
hops2json ${output_file}

#use jq (json query) to extract the plot_data element and pipe to file
echo "jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" > tee ./fdump.json"
jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" > ./fdump.json

echo "Running: fourfit3 -m 4 -c ./test0.cf -b AS -P RR ./${MK4_SCAN_DIR} set plot_data_dir ./chk3 "
time fourfit3 -m 1 -c ./test0.cf -b AS -P RR ./${MK4_SCAN_DIR} set plot_data_dir ./chk3 2>&1  | tee ./ff.out

compjsonpdd.py ./fdump.json ./chk3/104-1228-AS-B-RR.*
RET_VAL=$?

exit $RET_VAL
