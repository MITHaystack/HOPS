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
CM42H_DIR=105-1800
#NOTE: difx2hops must be run from within the difx directory
#This is because any relative paths specified in the difx.input files are
#relative to where the executable is run, not relative to the directory they
#are placed in. This is just a quirk of the difxio library. Normally, absolute
#paths are used, but that doesn't work outside of a correlator environment.
cd $EXP_DIR
echo "Running: SimpleFringeSearch -d ./${D2H_EXP_NUM}/${SCAN_DIR} -c ./cf_test3 -b GE -p XX"
SimpleFringeSearch -d ./${D2H_EXP_NUM}/${SCAN_DIR} -c ./cf_test3 -b GE -p XX


# D2M4_GE_FILE=$(ls ./${CM42H_DIR}/GE.*.cor)
# D2H_GE_FILE=$(ls ./${D2H_EXP_NUM}/${SCAN_DIR}/GE.*.cor)
# echo "Running: CompareCorFiles -a $D2H_GE_FILE -b $D2M4_GE_FILE"
# CompareCorFiles -a $D2H_GE_FILE -b $D2M4_GE_FILE
RET_VAL=$?
exit $RET_VAL
