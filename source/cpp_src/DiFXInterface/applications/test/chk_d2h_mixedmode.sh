#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/single_scan; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
SCAN_DIR=343-2000
CONTROL_FILE=cf_kt5343_notches

#NOTE: This test exercises difx2hops on data containing zoom-bands (a Kokee-Ties mixed mode experiment)
# difx2hops must be run from within the difx directory to use the --localdir (-l) option.
#This is to override any paths specified in the difx.input files to use relative paths to local files.
#only DiFX difxio > 2.7 supports --localdir! 

cd $EXP_DIR

export HOPS_PLOT_DATA_MASK=0x83FFFFFF

#clear any previous output
rm -rf $EXP_DIR/1111 $EXP_DIR/2222 $EXP_DIR/3333 $EXP_DIR/4444
rm -rf $EXP_DIR/chk1 $EXP_DIR/chk2
rm -f  $EXP_DIR/fdump1.json $EXP_DIR/fdump2.json

#mixed-mode (8MHz) data in hops4 format
echo "Running: difx2hops -i ./ -o ./1111 -e 1111 -s ./codes -b S 0 2000 -b X 2001 99999 -l -w 8 -m -2"
difx2hops -i ./ -o ./1111 -e 1111 -s ./codes -b S 0 2000 -b X 2001 99999 -l -w 8 -m -2
[ $? -ne 0 ] && { echo "difx2hops (1111) failed"; exit 1; }

#mixed-mode (8MHz) data in mark4 format
echo "Running: difx2hops -i ./ -o ./2222 -e 2222 -s ./codes -b S 0 2000 -b X 2001 99999 -l -w 8 -m -2 -k"
difx2hops -i ./ -o ./2222 -e 2222 -s ./codes -b S 0 2000 -b X 2001 99999 -l -w 8 -m -2 -k
[ $? -ne 0 ] && { echo "difx2hops (2222) failed"; exit 2; }

#VGOS-only channels from K2 autocorr (32MHz) data in hops4 format
echo "Running: difx2hops -i ./ -o ./3333 -e 3333 -s ./codes -b S 0 2000 -b X 2001 99999 -l -w 32 -m -2"
difx2hops -i ./ -o ./3333 -e 3333 -s ./codes -b S 0 2000 -b X 2001 99999 -l -w 32 -m -2
[ $? -ne 0 ] && { echo "difx2hops (3333) failed"; exit 3; }

#VGOS-only channels from K2 autocorr (32MHz) data in mark4 format
echo "Running: difx2hops -i ./ -o ./4444 -e 4444 -s ./codes -b S 0 2000 -b X 2001 99999 -l -w 32 -m -2 -k"
difx2hops -i ./ -o ./4444 -e 4444 -s ./codes -b S 0 2000 -b X 2001 99999 -l -w 32 -m -2 -k
[ $? -ne 0 ] && { echo "difx2hops (4444) failed"; exit 4; }

# ----- pair 1: HK / XR (8MHz mixed-mode) -----
echo "Running: fourfit4 -m 4 -c ./${CONTROL_FILE} -b HK -P XR ./1111/${SCAN_DIR}/"
output_file=$(fourfit4 -m 4 -c ./${CONTROL_FILE} -b HK -P XR ./1111/${SCAN_DIR}/ 2>&1 | awk '{print $NF}')
echo "fourfit4 output file: $output_file"

hops2json ${output_file}
jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" > ./fdump1.json

echo "Running: fourfit3 -m 4 -c ./${CONTROL_FILE} -b HK -P XR ./2222/${SCAN_DIR} set plot_data_dir ./chk1"
fourfit3 -m 4 -c ./${CONTROL_FILE} -b HK -P XR ./2222/${SCAN_DIR} set plot_data_dir ./chk1 2>&1 | tee ./ff1.out

PDD1=$(ls -1 ./chk1/${SCAN_DIR}-HK-* 2>/dev/null | head -n 1)
if [ -z "$PDD1" ]; then
    echo "No PDD file produced by fourfit3 for HK baseline"
    RET_VAL=5
else
    echo "Comparing: compjsonpdd.py -r 0.04 -x FringeRate(Hz),ResidRate(us/s),ResidRateError(us/s) ./fdump1.json $PDD1"
    compjsonpdd.py -r 0.04 -x 'FringeRate(Hz),ResidRate(us/s),ResidRateError(us/s)' ./fdump1.json $PDD1
    RC=$?
    [ $RC -ne 0 ] && RET_VAL=$RC
fi

# ----- pair 2: HH / XR (32MHz VGOS autocorr) -----
echo "Running: fourfit4 -m 4 -b HH -P XX ./3333/${SCAN_DIR}/"
output_file=$(fourfit4 -m 4 -b HH -P XX ./3333/${SCAN_DIR}/ 2>&1 | awk '{print $NF}')
echo "fourfit4 output file: $output_file"

hops2json ${output_file}
jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" > ./fdump2.json

echo "Running: fourfit3 -m 4 -b HH -P XX ./4444/${SCAN_DIR} set plot_data_dir ./chk2"
fourfit3 -m 4 -b HH -P XX ./4444/${SCAN_DIR} set plot_data_dir ./chk2 2>&1 | tee ./ff2.out

PDD2=$(ls -1 ./chk2/${SCAN_DIR}-HH-* 2>/dev/null | head -n 1)
if [ -z "$PDD2" ]; then
    echo "No PDD file produced by fourfit3 for HH baseline"
    RET_VAL=6
else
    #exclude a bunch of fields (delay/delay-rate) which for an auto-corr should be zero (relative accuracy comparison fails)
    echo "Comparing: compjsonpdd.py -r 0.04 -x FringeRate(Hz),ResidRate(us/s),ResidRateError(us/s),ResidMbd(us),PhaseDelay(usec),ResidMbdelay(usec),ResidPhdelay(usec),ResidMbdelayError(usec),GroupDelayModel(usec) ./fdump2.json $PDD2"
    compjsonpdd.py -r 0.04 -x 'FringeRate(Hz),ResidRate(us/s),ResidRateError(us/s),ResidMbd(us),PhaseDelay(usec),ResidMbdelay(usec),ResidPhdelay(usec),ResidMbdelayError(usec),GroupDelayModel(usec)' ./fdump2.json $PDD2
    RC=$?
    [ $RC -ne 0 ] && RET_VAL=$RC
fi

echo "Return value: $RET_VAL"
exit $RET_VAL
