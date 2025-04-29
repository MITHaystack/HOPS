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
cd $EXP_DIR
export HOPS_PLOT_DATA_MASK=0x83FFFFFF

if [ ! -d "./${D2H_EXP_NUM}" ]; then
    echo "difx2hops not run, using mark42hops converted data (105-1800a) for test"
    HOPS4_DIR="105-1800b"
fi

# echo "Running: fourfit4 -c ./cf_test5 -b GE -P I ./${D2H_EXP_NUM}/${SCAN_DIR}"
# time fourfit4 -c ./cf_test5 -b GE -P I ./${D2H_EXP_NUM}/${SCAN_DIR} | grep max555 | tee ./sfs.out

echo "Running: fourfit4 -m 4 -c ./cf_test5 -b GE -P I ./${D2H_EXP_NUM}/${HOPS4_DIR}/"
outfile=$( fourfit4 -m 4 -c ./cf_test5 -b GE -P I ./${D2H_EXP_NUM}/${HOPS4_DIR}/  2>&1)

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
echo "jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" | tee ./fdump.json"
jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" | tee ./fdump.json

echo "Running: fourfit3 -m 1 -t -c ./cf_test5 -b GE -P I ./${D2M4_EXP_NUM}/${SCAN_DIR}"
time fourfit3 -m 1 -t -c ./cf_test5 -b GE -P I ./${D2M4_EXP_NUM}/${SCAN_DIR} set plot_data_dir ./chk1 2>&1  | grep max555 | tee ./ff.out

#tolerance is 2.6% currently...need to work out the differences and reduce this
compjsonpdd.py -r 0.026 ./fdump.json ./chk1/105-1800-GE-X-Ixy*
RET_VAL=$?

exit $RET_VAL
