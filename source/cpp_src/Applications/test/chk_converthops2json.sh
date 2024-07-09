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
CM42H_DIR=105-1800a
REF_JSON_FILE="$DATADIR/GE.0VSI1M.cor.json"
NEW_JSON_FILE="$DATADIR/summary.json"
cd $EXP_DIR

echo "Running: mark42hops -i ./${D2M4_EXP_NUM}/${SCAN_DIR} -o ./${CM42H_DIR}"
mark42hops -i ./${D2M4_EXP_NUM}/${SCAN_DIR} -o ./${CM42H_DIR}

echo "Running: hops2json -i ./${CM42H_DIR}/GE.0VSI1M.cor -d 0 -o ${NEW_JSON_FILE}"
hops2json -i "./${CM42H_DIR}/GE.0VSI1M.cor" -d 0 -o ${NEW_JSON_FILE}

#compare the json output to the reference
@PY_EXE@ @CMAKE_CURRENT_BINARY_DIR@/hopsobjdata.py $REF_JSON_FILE $NEW_JSON_FILE
RET_VAL=$?

exit $RET_VAL
