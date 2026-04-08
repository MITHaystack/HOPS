#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh

if [ ! -d "$srcdir/dummy" ]; then
  mkdir -p "$srcdir/dummy"
fi

export DATADIR=`cd $srcdir/dummy; pwd`
RET_VAL=0
EXP_DIR=$DATADIR
cd $EXP_DIR
SNR_VAL=200
NCHAN=256
CHANW=64
NAPS=30
NSPECPTS=128

#run dummy_vis to generate fake visibilty data to test if we can process n_channels > 128 (here we test 256)
dummy_vis --seed 99 --snr ${SNR_VAL} -c ${NCHAN} -w ${CHANW} -n ${NAPS} -s ${NSPECPTS}

echo "Running: fourfit4 -m 4 -b AB -P XX ./"
output_file=$(fourfit4 -m 4 -b AB -P XX ./ 2>&1 | awk '{print $NF}')
echo "fourfit4 output file: $output_file"

#convert the fringe file to json
hops2json ${output_file}

#use jq (json query) to extract the plot_data element and pipe to file
echo "jq '.[].tags.parameters.fringe.snr | select( . != null )' "${output_file}.json" > ./fdump.json"
jq '.[].tags.parameters.fringe.snr | select( . != null )' "${output_file}.json" > ./fdump.json

#use jq (json query) to extract the plot_data element and pipe to file
echo "jq '.[].tags.parameters.config.nchannels | select( . != null )' "${output_file}.json" > ./nchan.json"
jq '.[].tags.parameters.config.nchannels | select( . != null )' "${output_file}.json" > ./nchan.json

NCHAN_USED=$(cat ./nchan.json)
echo "fourfit4 used: $NCHAN_USED channels."

#determine if the SNR is near the specified value of SNR_VAL
SNR=$(cat ./fdump.json)

result=$(echo "sqrt(($SNR - $SNR_VAL)^2) < 1.0" | bc -l)
echo "The calculated SNR of the simulated fringe = $SNR, expected: $SNR_VAL"
echo "Result ok? $result"  # prints 1 if true, 0 if false

#invert the logic for the return code (0 is normal, 1 is fail)
if [ "$result" -eq 1 ]; then
    exit 0
else
    exit 1
fi
