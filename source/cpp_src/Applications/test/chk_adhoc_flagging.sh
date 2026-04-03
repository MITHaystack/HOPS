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

###################
#create the adhoc flagging file
ahffile=${EXP_DIR}/104-1228.flag
# day 104 12:28:00 for 120s = 103*86400 + 12*3600 + 0*60, then
# divide by 86400 subtract 1 from d.o.y counter (indexed from 1)

#103.519444... to 103.520833 (almost)
cat > $ahffile <<EOF
* double x = 104-1. + 12./24. + 28./(24*60.);
103.519444444444 FF
* double x = 104-1. + 12./24. + 28./(24*60.) + 10./(24*3600.);
103.519560185185 80
* double x = 104-1. + 12./24. + 28./(24*60.) + 20./(24*3600.);
103.519675925926 FF
* double x = 104-1. + 12./24. + 28./(24*60.) + 30./(24*3600.);
103.519791666667 80
* double x = 104-1. + 12./24. + 28./(24*60.) + 40./(24*3600.);
103.519907407407 FF
* double x = 104-1. + 12./24. + 28./(24*60.) + 50./(24*3600.);
103.520023148148 80
* double x = 104-1. + 12./24. + 28./(24*60.) + 60./(24*3600.);
103.520138888889 FF
* double x = 104-1. + 12./24. + 28./(24*60.) + 70./(24*3600.);
103.52025462963 00
EOF
####################

export HOPS_PLOT_DATA_MASK=0x83FFFFFF

SET_STRING="adhoc_flag_file ${ahffile}"

echo "Running: fourfit4 -m 4 -c ./test2.cf -b AS -P RR ./${SCAN_DIR}/ set ${SET_STRING}"
output_file=$(fourfit4 -m 4 -c ./test2.cf -b AS -P RR ./${SCAN_DIR} set ${SET_STRING} 2>&1 | awk '{print $NF}')
echo "fourfit4 output file: $output_file"

#now run fourfit3 with the adhoc file
echo "Running: fourfit3 -m 4 -c ./test2.cf -b AS -P RR ./${MK4_SCAN_DIR} set ${SET_STRING} plot_data_dir ./chk_adhoc_flag"
fourfit3 -m 4 -c ./test2.cf -b AS -P RR ./${MK4_SCAN_DIR} set ${SET_STRING} plot_data_dir ./chk_adhoc_flag 2>&1  | tee ./ff.out

#convert the fringe file to json
hops2json ${output_file}

#use jq (json query) to extract the plot_data element and pipe to file
echo "jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" > ./fdump_adhoc_flag.json"
jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" > ./fdump_adhoc_flag.json

#now compare the results between fourfit3 and fourfit4, tolerance 3%
#(MBD residual has a ~2.3% algorithmic difference between fourfit3 and fourfit4)
compjsonpdd.py -r 0.03 ./fdump_adhoc_flag.json ./chk_adhoc_flag/104-1228-AS-B-RR.*
RET_VAL=$?

exit $RET_VAL
