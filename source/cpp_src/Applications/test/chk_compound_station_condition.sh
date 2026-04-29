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
echo "Executing command: fourfit4 -m 4 -c ./${CONTROL_FILE} -b GE -P XX \"${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/\""

# Capture the output file name directly from fourfit4's output
output_file_ge=$(fourfit4 -m 4 -c ./${CONTROL_FILE} -b GE -P XX "${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/" 2>&1 | awk '{print $NF}')
echo "fourfit4 output file for GE: $output_file_ge"

# Check if output_file_ge is empty
if [ -z "$output_file_ge" ]; then
    echo "FAIL: fourfit4 did not produce a valid output file for GE baseline."
    RET_VAL=1
    exit $RET_VAL # Exit early if fourfit4 failed
fi


# Convert to JSON and extract phase correction for G and E
hops2json ${output_file_ge}
PC_PHASE_G_X=$(jq -r '.[].tags.parameters.pc_phase_offset[] | select(.station_id == "G" and .pol == "X") | .pc_phase_offset' "${output_file_ge}.json")
PC_PHASE_G_Y=$(jq -r '.[].tags.parameters.pc_phase_offset[] | select(.station_id == "G" and .pol == "Y") | .pc_phase_offset' "${output_file_ge}.json")
PC_PHASE_E_X=$(jq -r '.[].tags.parameters.pc_phase_offset[] | select(.station_id == "E" and .pol == "X") | .pc_phase_offset' "${output_file_ge}.json")
PC_PHASE_E_Y=$(jq -r '.[].tags.parameters.pc_phase_offset[] | select(.station_id == "E" and .pol == "Y") | .pc_phase_offset' "${output_file_ge}.json")

echo "PC Phase for G (X-pol): $PC_PHASE_G_X"
echo "PC Phase for G (Y-pol): $PC_PHASE_G_Y"
echo "PC Phase for E (X-pol): $PC_PHASE_E_X"
echo "PC Phase for E (Y-pol): $PC_PHASE_E_Y"

# Verify that both G and E received the 10.0 phase offset for both polarizations
if [ -z "$PC_PHASE_G_X" ] || [ "$(echo "$PC_PHASE_G_X == 10.0" | bc)" -ne 1 ] || \
   [ -z "$PC_PHASE_G_Y" ] || [ "$(echo "$PC_PHASE_G_Y == 10.0" | bc)" -ne 1 ] || \
   [ -z "$PC_PHASE_E_X" ] || [ "$(echo "$PC_PHASE_E_X == 10.0" | bc)" -ne 1 ] || \
   [ -z "$PC_PHASE_E_Y" ] || [ "$(echo "$PC_PHASE_E_Y == 10.0" | bc)" -ne 1 ]; then
    echo "FAIL: Baseline GE - Expected 10.0 phase offset for G(X,Y) and E(X,Y)."
    RET_VAL=1
else
    echo "PASS: Baseline GE - Both G and E received 10.0 phase offset for both polarizations."
fi

# --- Test Case 2: Baseline GV (only G in condition, V is another station) ---
echo "--- Running fourfit4 for baseline GV (only G in condition) ---"
echo "Executing command: fourfit4 -m 4 -c ./${CONTROL_FILE} -b GV -P XX "${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/""
fourfit4_output_log_gv=$(mktemp)
fourfit4 -m 4 -c ./${CONTROL_FILE} -b GV -P XX "${EXP_DIR}/${D2H_EXP_NUM}/${HOPS4_DIR}/" 2>&1 | tee "${fourfit4_output_log_gv}"

output_file_gv=$(grep "fourfit:" "${fourfit4_output_log_gv}" | awk '{print $NF}')
echo "fourfit4 output file for GV: $output_file_gv"

if [ -z "$output_file_gv" ] || ! grep -q "fourfit:" "${fourfit4_output_log_gv}"; then
    echo "FAIL: fourfit4 did not produce a valid output file for GV baseline."
    RET_VAL=1
    rm -f "${fourfit4_output_log_gv}"
    exit $RET_VAL # Exit early if fourfit4 failed
fi
rm -f "${fourfit4_output_log_gv}"

# Convert to JSON and extract phase correction for G and V
hops2json ${output_file_gv}
PC_PHASE_G_X_GV=$(jq -r '.[].tags.parameters.pc_phase_offset[] | select(.station_id == "G" and .pol == "X") | .pc_phase_offset' "${output_file_gv}.json")
PC_PHASE_G_Y_GV=$(jq -r '.[].tags.parameters.pc_phase_offset[] | select(.station_id == "G" and .pol == "Y") | .pc_phase_offset' "${output_file_gv}.json")
PC_PHASE_V_X_GV=$(jq -r '.[].tags.parameters.pc_phase_offset[] | select(.station_id == "V" and .pol == "X") | .pc_phase_offset' "${output_file_gv}.json")
PC_PHASE_V_Y_GV=$(jq -r '.[].tags.parameters.pc_phase_offset[] | select(.station_id == "V" and .pol == "Y") | .pc_phase_offset' "${output_file_gv}.json")

echo "PC Phase for G (X-pol, GV baseline): $PC_PHASE_G_X_GV"
echo "PC Phase for G (Y-pol, GV baseline): $PC_PHASE_G_Y_GV"
echo "PC Phase for V (X-pol, GV baseline): $PC_PHASE_V_X_GV"
echo "PC Phase for V (Y-pol, GV baseline): $PC_PHASE_V_Y_GV"

# Verify G received 10.0 for both polarizations, and V received nothing
if [ -z "$PC_PHASE_G_X_GV" ] || [ "$(echo "$PC_PHASE_G_X_GV == 10.0" | bc)" -ne 1 ] || \
   [ -z "$PC_PHASE_G_Y_GV" ] || [ "$(echo "$PC_PHASE_G_Y_GV == 10.0" | bc)" -ne 1 ]; then
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
rm -f ${CONTROL_FILE} ${output_file_ge} ${output_file_ge}.json \
      ${output_file_gv} ${output_file_gv}.json

exit $RET_VAL
