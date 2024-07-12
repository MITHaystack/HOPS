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
TMP_FILE="tmp.txt"
cd $EXP_DIR

hops2keys ./${D2H_EXP_NUM}/${SCAN_DIR}/GE.*.cor > ./${TMP_FILE}

#check the the dumped keys contain the class uuid's for visibilities, weights, and tags
N_VIS=$(grep "a5c26065821b6dc92b06f780f8641d0e" ./${TMP_FILE} | wc -l)
N_WEIGHT=$(grep "f05838a616aa848562a57d5ace23e8d1" ./${TMP_FILE} | wc -l )
N_TAGS=$(grep "330c5f9889eaa350f8955c6e709a536c" ./${TMP_FILE} | wc -l )

echo "N visibility objects = $N_VIS"
echo "N weight objects = $N_WEIGHT"
echo "N tag objects = $N_TAGS"

RET_VAL=1
if [[ "$N_VIS" == "1" ]] && [[ "$N_WEIGHT" == "1" ]] && [[ "$N_TAGS" == "1" ]];
then
    RET_VAL=0
fi

echo "Test value is $RET_VAL (ok if =0)."

exit $RET_VAL
