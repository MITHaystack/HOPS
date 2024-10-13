#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/3741; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
D2M4_EXP_NUM=.
MK4_SCAN_DIR=190-1800a
SCAN_DIR=190-1800b
CONTROL_FILE=cf_3741_v10-combo
cd $EXP_DIR


export HOPS_PLOT_DATA_MASK=0x83FFFFFF

echo "Running: fourfit4 -m 4 -c ./${CONTROL_FILE} -b GN:X -P XR+YR ./${SCAN_DIR}/"
outfile=$(time fourfit4 -m 4 -c ./${CONTROL_FILE} -b GN:X -P XR+YR ./${SCAN_DIR}/  2>&1)

#parse the print out (fourfit4: <fringe_filename>) into just the fringe_filename
echo "$outfile"
old_IFS=$IFS
IFS=" "
set -- $outfile
IFS=$old_IFS
cmdname=$1
output_file=$2
echo "output file: $output_file"

#convert the fringe file to json
hops2json ${output_file}

#use jq (json query) to extract the plot_data element and pipe to file
echo "jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" > tee ./fdump.json"
jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" > ./fdump.json


echo "Running: fourfit3 -m 4 -c ./${CONTROL_FILE} -b GN:X -P XR+YR ./${MK4_SCAN_DIR} set plot_data_dir ./chk3 "
time fourfit3 -m 1 -c ./${CONTROL_FILE} -b GN:X -P XR+YR ./${MK4_SCAN_DIR} set plot_data_dir ./chk4 2>&1  | tee ./ff.out

#tolerance is 1% here...but phases quantities fail (~30 percent)
compjsonpdd.py -r 0.01 ./fdump.json ./chk4/190-1800a-GN-X*
RET_VAL=$?

exit $RET_VAL
