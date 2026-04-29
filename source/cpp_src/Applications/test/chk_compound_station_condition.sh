#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/vt9105; pwd` # Using vt9105 data as it has stations G and E

RET_VAL=0
EXP_DIR=$DATADIR
D2H_EXP_NUM=1111 # Assuming difx2hops data is in this dir
HOPS4_DIR=105-1800b # Directory for fourfit4 data
CONTROL_FILE=cf_compound_station_test.cf
cd $EXP_DIR

export HOPS_PLOT_DATA_MASK=0x83FFFFFF

# Create the control file
cat > ${CONTROL_FILE} <<EOF
# Control file for testing compound station conditions
# Applies a phase correction to stations G or E

if station G or station E
  pc_phase_offset_x 10.0
  pc_phase_offset_y 10.0
EOF

# --- Test Case 1: Baseline GE (both stations in condition) ---
echo "--- Running fourfit4 for baseline GE (both G and E in condition) ---"

# Run XX pol-product
echo "Executing command: fourfit4 -m 4 -c ./${CONTROL_FILE} -b GE -P XX -X 0 \"${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/\""
output_file_ge_xx=$(fourfit4 -m 4 -c ./${CONTROL_FILE} -b GE -P XX -X 0 "${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/" 2>&1 | awk '{print $NF}')
echo "fourfit4 output file for GE (XX): $output_file_ge_xx"

if [ -z "$output_file_ge_xx" ]; then
    echo "FAIL: fourfit4 did not produce a valid output file for GE baseline (XX)."
    RET_VAL=1
    exit $RET_VAL
fi

# Run YY pol-product
echo "Executing command: fourfit4 -m 4 -c ./${CONTROL_FILE} -b GE -P YY -X 0 \"${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/\""
output_file_ge_yy=$(fourfit4 -m 4 -c ./${CONTROL_FILE} -b GE -P YY -X 0 "${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/" 2>&1 | awk '{print $NF}')
echo "fourfit4 output file for GE (YY): $output_file_ge_yy"

if [ -z "$output_file_ge_yy" ]; then
    echo "FAIL: fourfit4 did not produce a valid output file for GE baseline (YY)."
    RET_VAL=1
    exit $RET_VAL
fi

# Convert to JSON and extract phase corrections from both runs
hops2json -d 3 ${output_file_ge_xx}
hops2json -d 3 ${output_file_ge_yy}

# XX run: G(X) is ref, E(X) is rem
PC_PHASE_G_X=$(jq -r '[.. | objects | select(has("ref_pcphase_offset_X")) | .ref_pcphase_offset_X] | if length > 0 then .[0] * 57.29577951308232 else empty end' "${output_file_ge_xx}.json")
PC_PHASE_E_X=$(jq -r '[.. | objects | select(has("rem_pcphase_offset_X")) | .rem_pcphase_offset_X] | if length > 0 then .[0] * 57.29577951308232 else empty end' "${output_file_ge_xx}.json")

# YY run: G(Y) is ref, E(Y) is rem
PC_PHASE_G_Y=$(jq -r '[.. | objects | select(has("ref_pcphase_offset_Y")) | .ref_pcphase_offset_Y] | if length > 0 then .[0] * 57.29577951308232 else empty end' "${output_file_ge_yy}.json")
PC_PHASE_E_Y=$(jq -r '[.. | objects | select(has("rem_pcphase_offset_Y")) | .rem_pcphase_offset_Y] | if length > 0 then .[0] * 57.29577951308232 else empty end' "${output_file_ge_yy}.json")

echo "PC Phase for G (X-pol): $PC_PHASE_G_X"
echo "PC Phase for G (Y-pol): $PC_PHASE_G_Y"
echo "PC Phase for E (X-pol): $PC_PHASE_E_X"
echo "PC Phase for E (Y-pol): $PC_PHASE_E_Y"

# Verify that both G and E received the 10.0 phase offset for both polarizations (tolerance 0.1 deg)
if [ -z "$PC_PHASE_G_X" ] || [ "$(echo "$PC_PHASE_G_X < 9.9 || $PC_PHASE_G_X > 10.1" | bc)" -eq 1 ] || \
   [ -z "$PC_PHASE_G_Y" ] || [ "$(echo "$PC_PHASE_G_Y < 9.9 || $PC_PHASE_G_Y > 10.1" | bc)" -eq 1 ] || \
   [ -z "$PC_PHASE_E_X" ] || [ "$(echo "$PC_PHASE_E_X < 9.9 || $PC_PHASE_E_X > 10.1" | bc)" -eq 1 ] || \
   [ -z "$PC_PHASE_E_Y" ] || [ "$(echo "$PC_PHASE_E_Y < 9.9 || $PC_PHASE_E_Y > 10.1" | bc)" -eq 1 ]; then
    echo "FAIL: Baseline GE - Expected 10.0 phase offset for G(X,Y) and E(X,Y)."
    RET_VAL=1
else
    echo "PASS: Baseline GE - Both G and E received 10.0 phase offset for both polarizations."
fi

# --- Test Case 2: Baseline GV (only G in condition, V is another station) ---
echo "--- Running fourfit4 for baseline GV (only G in condition) ---"

# Run XX pol-product
echo "Executing command: fourfit4 -m 4 -c ./${CONTROL_FILE} -b GV -P XX -X 0 \"${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/\""
output_file_gv_xx=$(fourfit4 -m 4 -c ./${CONTROL_FILE} -b GV -P XX -X 0 "${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/" 2>&1 | awk '{print $NF}')
echo "fourfit4 output file for GV (XX): $output_file_gv_xx"

if [ -z "$output_file_gv_xx" ]; then
    echo "FAIL: fourfit4 did not produce a valid output file for GV baseline (XX)."
    RET_VAL=1
    exit $RET_VAL
fi

# Run YY pol-product
echo "Executing command: fourfit4 -m 4 -c ./${CONTROL_FILE} -b GV -P YY -X 0 \"${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/\""
output_file_gv_yy=$(fourfit4 -m 4 -c ./${CONTROL_FILE} -b GV -P YY -X 0 "${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/" 2>&1 | awk '{print $NF}')
echo "fourfit4 output file for GV (YY): $output_file_gv_yy"

if [ -z "$output_file_gv_yy" ]; then
    echo "FAIL: fourfit4 did not produce a valid output file for GV baseline (YY)."
    RET_VAL=1
    exit $RET_VAL
fi

# Convert to JSON and extract phase corrections from both runs
hops2json -d 3 ${output_file_gv_xx}
hops2json -d 3 ${output_file_gv_yy}

# XX run: G(X) is ref, V(X) is rem
PC_PHASE_G_X_GV=$(jq -r '[.. | objects | select(has("ref_pcphase_offset_X")) | .ref_pcphase_offset_X] | if length > 0 then .[0] * 57.29577951308232 else empty end' "${output_file_gv_xx}.json")
PC_PHASE_V_X_GV=$(jq -r '[.. | objects | select(has("rem_pcphase_offset_X")) | .rem_pcphase_offset_X] | if length > 0 then .[0] * 57.29577951308232 else empty end' "${output_file_gv_xx}.json")

# YY run: G(Y) is ref, V(Y) is rem
PC_PHASE_G_Y_GV=$(jq -r '[.. | objects | select(has("ref_pcphase_offset_Y")) | .ref_pcphase_offset_Y] | if length > 0 then .[0] * 57.29577951308232 else empty end' "${output_file_gv_yy}.json")
PC_PHASE_V_Y_GV=$(jq -r '[.. | objects | select(has("rem_pcphase_offset_Y")) | .rem_pcphase_offset_Y] | if length > 0 then .[0] * 57.29577951308232 else empty end' "${output_file_gv_yy}.json")

echo "PC Phase for G (X-pol, GV baseline): $PC_PHASE_G_X_GV"
echo "PC Phase for G (Y-pol, GV baseline): $PC_PHASE_G_Y_GV"
echo "PC Phase for V (X-pol, GV baseline): $PC_PHASE_V_X_GV"
echo "PC Phase for V (Y-pol, GV baseline): $PC_PHASE_V_Y_GV"

# Verify G received 10.0 for both polarizations, and V received nothing (tolerance 0.1 deg)
if [ -z "$PC_PHASE_G_X_GV" ] || [ "$(echo "$PC_PHASE_G_X_GV < 9.9 || $PC_PHASE_G_X_GV > 10.1" | bc)" -eq 1 ] || \
   [ -z "$PC_PHASE_G_Y_GV" ] || [ "$(echo "$PC_PHASE_G_Y_GV < 9.9 || $PC_PHASE_G_Y_GV > 10.1" | bc)" -eq 1 ]; then
    echo "FAIL: Baseline GV - Expected 10.0 phase offset for G(X,Y)."
    RET_VAL=1
elif [ -n "$PC_PHASE_V_X_GV" ] && [ "$(echo "$PC_PHASE_V_X_GV != 0.0" | bc)" -eq 1 ]; then
    echo "FAIL: Baseline GV - Expected no phase offset for V(X), got $PC_PHASE_V_X_GV"
    RET_VAL=1
elif [ -n "$PC_PHASE_V_Y_GV" ] && [ "$(echo "$PC_PHASE_V_Y_GV != 0.0" | bc)" -eq 1 ]; then
    echo "FAIL: Baseline GV - Expected no phase offset for V(Y), got $PC_PHASE_V_Y_GV"
    RET_VAL=1
else
    echo "PASS: Baseline GV - G received 10.0 phase offset for both polarizations, V received none."
fi

# Clean up generated files
rm -f ${CONTROL_FILE} \
      ${output_file_ge_xx} ${output_file_ge_xx}.json \
      ${output_file_ge_yy} ${output_file_ge_yy}.json \
      ${output_file_gv_xx} ${output_file_gv_xx}.json \
      ${output_file_gv_yy} ${output_file_gv_yy}.json

exit $RET_VAL
