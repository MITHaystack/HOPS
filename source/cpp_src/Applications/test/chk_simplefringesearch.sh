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

echo "Running: fourfit4 -m 4 -c ./cf_test4 -b GE -P XX ./${D2H_EXP_NUM}/${HOPS4_DIR}/"
outfile=$(fourfit4 -m 4 -c ./cf_test4 -b GE -P XX ./${D2H_EXP_NUM}/${HOPS4_DIR}/ 2>&1)

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

#run fourfit and dump its data to a 'plot_data_dir' file
echo "Running: fourfit3 -t -c ./cf_test4 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR}"
time fourfit3 -t -c ./cf_test4 -b GE -P XX ./${D2M4_EXP_NUM}/${SCAN_DIR} set plot_data_dir ./chk1 2>&1

#finally compare the outputs
compjsonpdd.py ./fdump.json ./chk1/105-1800-GE-X-XX*
RET_VAL=$?

exit $RET_VAL
