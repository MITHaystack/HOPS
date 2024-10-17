#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh

if [ ! -d "$srcdir/3768" ]; then
  echo "Directory $srcdir/3768 does not exist, will not apply this test"
  exit 127
fi

export DATADIR=`cd $srcdir/3768; pwd`
RET_VAL=0
EXP_DIR=$DATADIR
SCAN_DIR=108-0231a
cd $EXP_DIR

export HOPS_PLOT_DATA_MASK=0x83FFFFFF

echo "Running: fourfit4 -c ./cf_test_jmp -s 10  -P RR -b KN ./${SCAN_DIR}"
outfile=$( fourfit4 -m 4 -c ./cf_test_jmp -s 10  -P RR -b KN ./${SCAN_DIR} 2>&1)
#parse the print out (fourfit4: <fringe_filename>) into just the fringe_filename
echo "$outfile"
old_IFS=$IFS
IFS=" "
set -- $outfile
IFS=$old_IFS
cmdname=$1
outfile_rr=$2
echo "output RR file: $outfile_rr"

#convert the fringe file to json and use jq to pull out the amplitude and snr
hops2json ${outfile_rr}
jq '.[].tags.parameters.fringe.famp | select( . != null )' "${outfile_rr}.json" | tee ./famp_rr.txt
jq '.[].tags.parameters.fringe.snr | select( . != null )' "${outfile_rr}.json" | tee ./snr_rr.txt

echo "Running: fourfit4 -c ./cf_test_jmp -s 10  -P LL -b KN ./${SCAN_DIR}"
outfile=$( fourfit4 -m 4 -c ./cf_test_jmp -s 10  -P LL -b KN ./${SCAN_DIR} 2>&1)
#parse the print out (fourfit4: <fringe_filename>) into just the fringe_filename
echo "$outfile"
old_IFS=$IFS
IFS=" "
set -- $outfile
IFS=$old_IFS
cmdname=$1
outfile_ll=$2
echo "output LL file: $outfile_ll"

hops2json ${outfile_ll}
jq '.[].tags.parameters.fringe.famp | select( . != null )' "${outfile_ll}.json" | tee ./famp_ll.txt
jq '.[].tags.parameters.fringe.snr | select( . != null )' "${outfile_ll}.json" | tee ./snr_ll.txt

echo "Running: fourfit4 -c ./cf_test_jmp -s 10  -P RR+LL -b KN ./${SCAN_DIR}"
outfile=$( fourfit4 -m 4 -c ./cf_test_jmp -s 10  -P RR+LL -b KN ./${SCAN_DIR} 2>&1)
#parse the print out (fourfit4: <fringe_filename>) into just the fringe_filename
echo "$outfile"
old_IFS=$IFS
IFS=" "
set -- $outfile
IFS=$old_IFS
cmdname=$1
outfile_combo=$2
echo "output RR+LL file: $outfile_combo"

hops2json ${outfile_combo}
jq '.[].tags.parameters.fringe.famp | select( . != null )' "${outfile_combo}.json" | tee ./famp_combo.txt
jq '.[].tags.parameters.fringe.snr | select( . != null )' "${outfile_combo}.json" | tee ./snr_combo.txt

ll_snr=$( cat ./snr_ll.txt) 
rr_snr=$( cat ./snr_rr.txt)
combo_snr=$(cat ./snr_combo.txt)

echo "LL snr: $ll_snr"
echo "RR snr: $rr_snr"
echo "RR+LL snr: $combo_snr"

#check that the SNR's are about where they should be
aok_ll_snr=$(echo "$ll_snr>110.0" | bc)
aok_rr_snr=$(echo "$rr_snr>110.0" | bc)
aok_combo_snr=$(echo "$combo_snr>155.0" | bc)

#compute individual pol-product SNR's w.r.t to RR+LL
ll_ratio=$(echo "scale=14; (($combo_snr)/$ll_snr)" | bc)
rr_ratio=$(echo "scale=14; (($combo_snr)/$rr_snr)" | bc)

echo "RR+LL to LL snr ratio = $ll_ratio"
echo "RR+LL to RR snr ratio = $rr_ratio"

#check that the SNR ratios are near sqrt(2)
SQRT2=1.4142136
low=0.98
high=0.995
echo "Tolerance on SNR ratio is (0.98 < x/sqrt(2) < 0.995)"
aok_ll=$(echo "$ll_ratio>($SQRT2)*($low) && $ll_ratio<($SQRT2)*($high)" | bc)
aok_rr=$(echo "$rr_ratio>($SQRT2)*($low) && $rr_ratio<($SQRT2)*($high)" | bc)

RET_VAL=1
if [ "$aok_ll_snr" -eq 1 -a "$aok_rr_snr" -eq 1 -a "$aok_combo_snr" -eq 1 -a "$aok_ll" -eq 1 -a "$aok_rr" -eq 1 ]; then
    RET_VAL=0
fi

exit $RET_VAL
