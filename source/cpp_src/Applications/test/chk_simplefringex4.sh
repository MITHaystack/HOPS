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


#step one, clean up any leftover fringe files
rm ${EXP_DIR}/1111/105-1800b/*.frng
rm ${EXP_DIR}/1234/105-1800/??.X.*.*

#run fourfit4
echo "Running: fourfit4 -m 4 -c ./cf_3686_GEHSVY_pstokes2 -b GE -P YY ./${D2H_EXP_NUM}/${HOPS4_DIR}/"
outfile4=$(fourfit4 -m 4 -c ./cf_3686_GEHSVY_pstokes2 -b GE -P YY ./${D2H_EXP_NUM}/${HOPS4_DIR}/ 2>&1)

#parse the print out (fourfit4: <fringe_filename>) into just the fringe_filename
echo "$outfile4"
old_IFS=$IFS
IFS=" "
set -- $outfile4
IFS=$old_IFS
cmdname=$1
output_file4=$2
echo "fourfit4 output file: $output_file4"

#run fourfit and dump its data to a 'plot_data_dir' file
echo "Running: fourfit3 -m 4 -c ./cf_3686_GEHSVY_pstokes2 -b GE -P YY ./${D2M4_EXP_NUM}/${SCAN_DIR}"
outfile3=$(fourfit3 -m 4 -c ./cf_3686_GEHSVY_pstokes2 -b GE -P YY ./${D2M4_EXP_NUM}/${SCAN_DIR} 2>&1)

#parse the print out (fourfit4: <fringe_filename>) into just the fringe_filename
echo "$outfile3"
old_IFS=$IFS
IFS=" "
set -- $outfile3
IFS=$old_IFS
cmdname=$1
output_file3=$2
echo "fourfit3 output file: $output_file3"

# #run fringex4 on the fourfit4 ouput
# fringex4 -o -v 5 -i 10 ${output_file4} > ${EXP_DIR}/fringex4.alist
# 
# #run fringex (original) on the fourfit3 output
# fringex -o -v 5 -i 10 ${output_file3} > ${EXP_DIR}/fringex3.alist

#run fringex4 on the fourfit4 ouput
fringex4 -v 5 -i 10 ${output_file4} > ${EXP_DIR}/fringex4.alist

#run fringex (original) on the fourfit3 output
fringex -v 5 -i 10 ${output_file3} > ${EXP_DIR}/fringex3.alist


#compare the two alist outputs
@PY_EXE@ @CMAKE_CURRENT_BINARY_DIR@/compare_alist.py \
    ${EXP_DIR}/fringex4.alist \
    ${EXP_DIR}/fringex3.alist \
    --verbose
RET_VAL=$?

exit $RET_VAL
