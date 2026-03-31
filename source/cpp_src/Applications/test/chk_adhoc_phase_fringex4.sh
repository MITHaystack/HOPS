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

echo "Running: fourfit4 -m 4 -c ./test2.cf -b AS -P RR ./${SCAN_DIR}/"
outfile=$(time fourfit4 -m 4 -c ./test2.cf -b AS -P RR ./${SCAN_DIR}/  2>&1)

#basic procedure to generate adhoc_phase file for consumption by fourfit3 (note: 3!)
#fringex4 -i 1 -v 6 ./104-1228a/AS.Aa-Sw.X.RR.2GAB6T.1.frng -O ./out.alist
#adhoc-baseline.pl -a ./out.alist  -b AS 
#in general case, call:
#print_adhoc_lines.pl
#to generate control file lines, 
#then tweak the new control file (test_adhoc_phase.cf) to point to the adhoc files:
#fourfit -c ./test_adhoc_phase.cf -P RR -b AS -pt ./104-1228/

#just for this test, we also have the silly step of reversing the channel labels
#in the control file test...because the control files in this 
#directory are also testing the freq <-> channel labelling feature

#parse the print out (fourfit4: <fringe_filename>) into just the fringe_filename
echo "$outfile"
old_IFS=$IFS
IFS=" "
set -- $outfile
IFS=$old_IFS
cmdname=$1
output_file=$2
echo "output file: $output_file"

#call fringex4 with 1 second bins on the new fringe file to generate an alist file
echo "fringex4 -i 1 -v 6 ${output_file} -O ./out.alist"
fringex4 -i 1 -v 6 ${output_file} -O ./out.alist

#generate the adhoc phase files for stations A & S with Vincent's perl script 
echo "adhoc-baseline.pl -o adhoc_phase_test -a ./out.alist  -b AS"
adhoc-baseline.pl -o adhoc_phase_test -a ./out.alist -b AS

#generate the control file lines 
echo "print_adhoc_lines.pl -b adhoc_phase_test -s AS"
print_adhoc_lines.pl -b adhoc_phase_test -s AS

#do in-place sed replacement of the channel ordering labels 
#This is not normal procedure! But rather due to the peculiarities of this test directory
#and how test2.cf was put together
sed -i 's/abcdefghijklmnopqrstuvwxyzABCDEF/FEDCBAzyxwvutsrqponmlkjihgfedcba/g' adhoc_lines.txt

#concatenate test2.cf and adhoc_lines.txt into test_adhoc.cf
cat test2.cf adhoc_lines.txt > test_adhoc.cf

#now run fourfit3 with the adhoc file 
echo "Running: fourfit3 -m 4 -c ./test_adhoc.cf -b AS -P RR ./${MK4_SCAN_DIR} set plot_data_dir ./chk_adhoc "
time fourfit3 -m 1 -c ./test_adhoc.cf -b AS -P RR ./${MK4_SCAN_DIR} set plot_data_dir ./chk_adhoc 2>&1  | tee ./ff.out

#grab the SNR value
SNR=$(grep "fourfit: SNR" ./ff.out | awk '{print $NF}')
result=$(echo "($SNR > 120) && ($SNR < 125)" | bc)
echo "The SNR of the adhoc'd fringe = $SNR"
echo "Result ok (121 < SNR < 125)? $result"  # prints 1 if true, 0 if false

#invert the logic for the return code (0 is normal, 1 is fail)
if [ "$result" -eq 1 ]; then
    exit 0
else
    exit 1
fi
