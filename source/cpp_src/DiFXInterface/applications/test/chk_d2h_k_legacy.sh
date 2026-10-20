#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/vt9105; pwd`

#checks that difx2hops -k -L export works (proper channel ids generated)
EXP_DIR=$DATADIR
EXP_NUM=4444
SCAN_DIR=105-1800
#NOTE difx2hops must be run from within the difx directory
cd $EXP_DIR
rm -rf $EXP_DIR/$EXP_NUM/
difx2hops -k -L -e $EXP_NUM -s ./station.codes -i ./ || { echo "difx2hops failed"; exit 1; }

FF_LOG=$EXP_DIR/$EXP_NUM/fourfit3.log
fourfit3 -m 1 -c ./cf_3686_GEHSVY_pstokes -b GE -P I ./$EXP_NUM/$SCAN_DIR > $FF_LOG 2>&1

if grep -q "does not appear in root" $FF_LOG; then
    echo "fourfit3 could not match t101 channel ids to root chan_defs:"
    grep "does not appear in root" $FF_LOG | head -3
    exit 2
fi

RET_VAL=0
for BAND in S C X; do
    if ls ./$EXP_NUM/$SCAN_DIR/GE.$BAND.*.* > /dev/null 2>&1; then
        echo "Found GE $BAND-band fringe file."
    else
        echo "Missing GE $BAND-band fringe file."
        RET_VAL=3
    fi
done

[ "$RET_VAL" == "0" ] && rm -rf $EXP_DIR/$EXP_NUM/
echo "Return value: $RET_VAL"
exit $RET_VAL
