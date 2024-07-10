#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/vt9105; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
cd $EXP_DIR


#the round trip of vex -> json -> vex -> json -> vex 
#should produce two identical vex files (with comments stripped)
echo "Running: vex2json -i ./vt9105.vex.obs -o ./test1.json"
vex2json -i ./vt9105.vex.obs -o ./test1.json

echo "Running: json2vex -i ./test1.json -o ./test1.vex.proxy"
json2vex -i ./test1.json -o ./test1.vex.proxy

echo "Running: vex2json -i ./test1.vex -o ./test2.json"
vex2json -i ./test1.vex.proxy -o ./test2.json

echo "Running: json2vex -i ./test2.json -o ./test2.vex.proxy"
json2vex -i ./test2.json -o ./test2.vex.proxy

#should return 0 if there is no difference between the two converted files
diff ./test1.vex.proxy ./test2.vex.proxy

RET_VAL=$?

exit $RET_VAL
