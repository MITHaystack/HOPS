#!/bin/bash
# Exercise the no-embed (subprocess) default fringe plotter against a real,
# captured plot_dict. We reuse the same fourfit4 -> hops2json -> jq dump that
# chk_simplefringesearch4.sh performs to produce fdump.json (the plot_data of a
# GE/I fringe fit), then hand that fixture to the installed
# TestSubprocessPythonPlotVisitor binary, which renders it through the python3
# subprocess and asserts a real image file is written.
#
# Exit 127 (this suite's SKIP code) when the test binary, jq, or the python
# plotting deps are unavailable; otherwise propagate the binary's pass/fail.
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/vt9105; pwd`

# the subprocess-plotter unit-test binary is installed under $HOPS_INSTALL/bin/test
TEST_BIN="$HOPS_INSTALL/bin/test/TestSubprocessPythonPlotVisitor"
if [ ! -x "$TEST_BIN" ]; then
    echo "subprocess plotter test binary not found ($TEST_BIN); skipping"
    exit 127
fi

# we need jq to extract the plot_data fixture from the hops2json output
if ! command -v jq >/dev/null 2>&1; then
    echo "jq not available; skipping"
    exit 127
fi

EXP_DIR=$DATADIR
D2H_EXP_NUM=1111
SCAN_DIR=105-1800
HOPS4_DIR=105-1800b
cd $EXP_DIR
export HOPS_PLOT_DATA_MASK=0x83FFFFFF

if [ ! -d "./${D2H_EXP_NUM}" ]; then
    echo "difx2hops not run, using mark42hops converted data (105-1800a) for test"
    HOPS4_DIR="105-1800b"
fi

FIXTURE="$DATADIR/fdump.json"

echo "Running: fourfit4 -m 4 -c ./cf_test5 -b GE -P I ./${D2H_EXP_NUM}/${HOPS4_DIR}/"
output_file=$(fourfit4 -m 4 -c ./cf_test5 -b GE -P I ./${D2H_EXP_NUM}/${HOPS4_DIR}/ 2>&1 | awk '{print $NF}')
echo "fourfit4 output file: $output_file"

if [ -z "$output_file" ] || [ ! -f "$output_file" ]; then
    echo "fourfit4 did not produce a fringe file; skipping"
    exit 127
fi

# convert the fringe file to json, then extract the plot_data element to fdump.json
hops2json ${output_file}
echo "jq '.[].tags.plot_data | select( . != null )' ${output_file}.json | tee $FIXTURE"
jq '.[].tags.plot_data | select( . != null )' "${output_file}.json" | tee "$FIXTURE" >/dev/null

if [ ! -s "$FIXTURE" ]; then
    echo "failed to dump plot_data fixture ($FIXTURE); skipping"
    exit 127
fi

echo "Running: $TEST_BIN $FIXTURE"
"$TEST_BIN" "$FIXTURE"
RET_VAL=$?

# the binary uses 77 for SKIP (python/deps unavailable); map it onto this
# suite's SKIP_RETURN_CODE (127) so an incapable environment is not a failure.
if [ "$RET_VAL" -eq 77 ]; then
    echo "subprocess plotter test skipped (python plotting deps unavailable)"
    exit 127
fi

exit $RET_VAL
