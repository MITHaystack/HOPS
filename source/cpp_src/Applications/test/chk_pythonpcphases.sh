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
export HOPS_PLOT_DATA_MASK=0x83FFFFFF

if [ ! -d "./${D2H_EXP_NUM}" ]; then
    echo "difx2hops not run, using mark42hops converted data (105-1800a) for test"
    HOPS4_DIR="105-1800b"
fi

echo "Running: fourfit4 -c ./cf_3686_GEHSVY_pstokes2 -b GV -P XY -m 5 ./${D2H_EXP_NUM}/${HOPS4_DIR}/ set python_finalize example3 generate_pcphases"
pcphases=$(fourfit4 -c ./cf_3686_GEHSVY_pstokes2 -b GV -P XY -m 5 ./${D2H_EXP_NUM}/${HOPS4_DIR}/ set python_finalize example3 generate_pcphases)

#just verify that we generated and output string with the appropriate number of tokens 
echo "pcphase statement generated: $pcphases"
ntokens=$(echo "$pcphases" | wc -w)

RET_VAL=1
if [[ "$ntokens" -eq 37 ]]; then
    RET_VAL=0
fi

exit $RET_VAL
