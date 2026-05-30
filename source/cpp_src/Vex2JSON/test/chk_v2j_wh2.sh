#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
# chk_env.sh puts vex2json/json2vex (and their runtime libs) on the path
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh

# This test is self-contained on its input: it uses the in-repo VEX 2.0 example
# fixture (data/wh2_ascii) rather than the external DiFX test data, and runs the
# conversion round trip in a scratch dir under the build tree.
FIXTURE_DIR="@CMAKE_CURRENT_SOURCE_DIR@/data"
WORK_DIR="@CMAKE_CURRENT_BINARY_DIR@/wh2_v2j_output"

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR" || { echo "could not create work dir $WORK_DIR"; exit 1; }
cp "$FIXTURE_DIR/wh2_ascii" "$WORK_DIR/wh2_ascii" || { echo "missing fixture $FIXTURE_DIR/wh2_ascii"; exit 1; }
cd "$WORK_DIR"

RET_VAL=0

#exercise the help flags
vex2json -h
json2vex -h

#the round trip of vex -> json -> vex -> json -> vex
#once normalized by the first json2vex pass, subsequent round trips should be
#stable, so the two proxy vex files (with comments stripped) must be identical.
echo "Running: vex2json -i ./wh2_ascii -o ./test1.json"
vex2json -m -2 -i ./wh2_ascii -o ./test1.json

echo "Running: json2vex -i ./test1.json -o ./test1.vex.proxy"
json2vex -m -2 -i ./test1.json -o ./test1.vex.proxy

echo "Running: vex2json -i ./test1.vex.proxy -o ./test2.json"
vex2json -m -2 -i ./test1.vex.proxy -o ./test2.json

echo "Running: json2vex -i ./test2.json -o ./test2.vex.proxy"
json2vex -m -2 -i ./test2.json -o ./test2.vex.proxy

#should return 0 if there is no difference between the two converted files
echo "Comparing: test1.vex.proxy vs test2.vex.proxy"
diff ./test1.vex.proxy ./test2.vex.proxy

RET_VAL=$?

exit $RET_VAL
