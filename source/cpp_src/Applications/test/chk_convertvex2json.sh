#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/vt9105; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
VEX_FILE="$DATADIR/vt9105.vex"
REF_JSON_FILE="$DATADIR/vt9105.vex.json"
NEW_JSON_FILE="$DATADIR/tmp.json"
cd $EXP_DIR

#convert the vex file to a json file 
echo "Running: ConvertVexToJSON -i $VEX_FILE -o $NEW_JSON_FILE"
ConvertVexToJSON -i $VEX_FILE -o $NEW_JSON_FILE

#compare the json output to the reference 
@PY_EXE@ @CMAKE_CURRENT_BINARY_DIR@/compjson.py $REF_JSON_FILE $NEW_JSON_FILE
EQUIV=$?
echo "Test value is $EQUIV (same if =0)."

exit $EQUIV
