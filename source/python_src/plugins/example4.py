import pyMHO_Containers
import pyMHO_Operators
import pyMHO_Calibration  # must import so pybind11 can downcast MHO_Operator* to the correct type

def set_pc_phase_offset_y(fringe_data_interface):
    """
    Example: retrieve the pc_phase_offset_y calibration operator from the toolbox
    and override its phase offset to 90 degrees.

    The operator is an MHO_ManualPolPhaseCorrection instance created by the builder
    when the control file contains a 'pc_phase_offset_y' statement.  Its toolbox
    key is always the literal string "pc_phase_offset_y".

    After changing configuration, initialize() must be called so the operator
    re-caches any pre-computed values before Execute() runs.
    """
    toolbox = fringe_data_interface.get_operator_toolbox()

    if toolbox is None:
        print("example4: operator toolbox not available")
        return

    print("example4: operators in toolbox:")
    for name in toolbox.get_operator_names():
        print(f"  {name}")

    # Retrieve the all the operator which share a name.  Because pyMHO_Calibration is imported,
    # pybind11 downcasts the returned MHO_Operator* to MHO_ManualPolPhaseCorrection.
    # However, multiple copies may have been created by the control file machinery, so we need to inspect
    # these for the one which matches the station we are interested in
    station_mk4_id = "E" #westford
    station_id = "Wf" #westford
    ops = toolbox.get_all_operators_by_name("pc_phase_offset_y")

    if len(ops) == 0:
        print("example4: 'pc_phase_offset_y' operator not found in toolbox "
              "(check that the control file contains a pc_phase_offset_y statement)")
        return

    ph_off = 140.0
    for op in ops:
        stid = op.get_station_identifier()
        if stid == station_mk4_id or stid == station_id:
            print(f"example4: found operator '{op.get_name()}', for station: '{stid}', setting phase offset to '{ph_off}'")
            op.set_pc_phase_offset(ph_off)
            # Re-initialize so the new offset is picked up before Execute() is called.
            ok = op.initialize()
            if not ok:
                print("example4: warning - initialize() returned False after reconfiguration")
