MHO\_EstimatePCManual
=====================

Purpose
-------
``MHO_EstimatePCManual`` estimates manual phase calibration and delay offset
values from post-fringe-fit residuals for stations configured
in ``pc_mode = manual``. It computes per-channel phase corrections
(``pc_phases``), per-channel delay offsets (``delay_offs``), and an overall
phase offset (``pc_phase_offset``) that can be used to refine the control file
for a subsequent fringe-fit pass. This is the HOPS4 port of the legacy fearfit
manual estimation code.

Control File Trigger
--------------------
- **Keyword:** ``est_pc_manual`` (integer parameter, not an operator keyword)
- **Category:** manual phase calibration estimation (a descriptive label used only in
  this document; this operator is not registered under any real pipeline
  ``operator_category``, see below)

.. list-table:: Bitmask modes for ``est_pc_manual``
   :header-rows: 1

   * - Bitmask
     - Hex
     - Action
   * - Estimate phases
     - 0x001
     - Per-channel phase correction.
   * - Median channel SBD
     - 0x002
     - Use median channel single-band delay.
   * - Average channel SBD
     - 0x004
     - Use average channel single-band delay.
   * - Total SBD
     - 0x008
     - Use total SBD channel delay.
   * - Measured SBD
     - 0x010
     - Use original measured SBD values.
   * - Outlier rejection
     - 0x020
     - Discard SBD outliers using median-based heuristics.
   * - Estimate phase offset
     - 0x040
     - Estimate overall phase offset.
   * - Phase bias (HOPS_EST_PC_BIAS)
     - 0x080
     - Apply phase bias from environment variable.
   * - Post-MDLY SBD adjustment
     - 0x100
     - Adjust SBD after multi-band delay.
   * - Estimate on reference station
     - sign bit
     - When set, also estimate on reference station.

There is no dedicated builder for this operator. It is instantiated directly and
 driven by the ``est_pc_manual`` parameter value.

Input Data
----------
This operator is an inspecting operator on ``visibility_type``. It additionally
requires:

- ``weight_type`` for per-channel, per-AP weights.
- ``phasor_type`` containing the fringe-fit residual phasors.
- ``MHO_ParameterStore`` for control-file parameters and fringe-fit results.
- Plot data (for SBD box values read from ``/PLOT_INFO``).

Algorithm
---------
``MHO_EstimatePCManual`` has a trivial ``InitializeImpl`` (always returns true); all work occurs in ``ExecuteImpl``.

**Execution (``ExecuteImpl``):**

1. Retrieve the ``est_pc_manual`` mode integer from the parameter store and delegate to ``est_pc_manual(mode)``.
2. **Mode setup:** Retrieve station IDs, polarization products, and per-station ``pc_mode``.
   The operator only proceeds if both reference and remote stations are in ``pc_mode = "manual"``.
   Decode the mode bitmask to determine which sub-tasks to run: phase estimation (``dophs = mode & 0x001``),
   delay estimation (``dodly = mode & 0x13e``), phase offset (``dooff = mode & 0x040``),
   and phase bias (``domrp = mode & 0x080``). The sign bit determines whether to
   estimate on the reference station.

**Phase Estimation (``est_phases``):**

1. Compute the weighted average phasor per channel: ``phi_avg = (1 / sum(w)) * sum(w_j * Phi_j)``, where ``Phi_j`` is the phasor for AP ``j``.
2. Extract the residual phase: ``theta_resid = arg(phi_avg)``.
3. For each channel (sorted by increasing frequency):

   a. Retrieve the net sideband label to determine the sideband multiplier (``sbmult = -1`` for LSB, ``+1`` for USB).
   b. Retrieve existing manual phase calibrations for both reference and remote stations: ``est_phase = ref_pc - rem_pc``.
   c. Compute the delay correction from fringe residuals: ``delta_delay = resid_mbd`` when ``mbd_anchor = "model"``; ``resid_mbd - resid_sbd`` when ``mbd_anchor = "sbd"``; ``0`` otherwise.
   d. Combine: ``est_phase = est_phase + sbmult * theta_resid * (180/pi) + 360 * delta_delay * (f_chan - f_ref)``.
   e. Optionally apply phase bias from ``HOPS_EST_PC_BIAS`` environment variable.
   f. Canonicalize to principal branch ``[-180, 180]`` degrees and adjust relative to the input phase for the target station.

4. Log the resulting control-file lines with station ID, polarization, channel labels, and estimated phases.

**Delay Estimation (``est_delays``):**

1. Read SBD box values from the plot data (``/PLOT_INFO/SbdBox``).
2. Convert SBD values from lag units to nanoseconds: ``sbd[ch] = (sbd[ch] - N_lags - 1) * sbd_sep * 1000``.
3. Negate for the remote station.
4. Call ``adj_delays`` which applies one of five methods based on the ``how`` bitmask:

   - ``0x02``: Use the median channel SBD value.
   - ``0x04``: Use the average channel SBD value.
   - ``0x08``: Use the total SBD value.
   - ``0x10``: Use the measured per-channel SBD values.
   - ``0x20``: Replace outliers (deviation > 3 sigma from median) with the median before computing the average.

5. Subtract the delta delay and negate for the remote station. Add the existing manual delay offsets and log the control-file lines.

**Phase Offset Estimation (``est_offset``):**

1. Retrieve the fringe-fit residual phase from the parameter store.
2. Retrieve the existing ``pcphase_offset`` for the target station (or use 0 if absent).
3. Compute the offset: ``ofs = resphase - pcphase_offset`` for the reference station, or ``ofs = -resphase + pcphase_offset`` for the remote station.
4. Log the control-file line.

Effect on Data
--------------
This operator does not modify the visibility or weight containers.
It inspects the phasors, weights, and fringe-fit residuals to compute estimated
phase calibration and delay offset values, which are logged as informational
control-file lines. The caller can use these values to update
the ``pc_phases``, ``delay_offs``, and ``pc_phase_offset`` parameters for a
subsequent fringe-fit pass.
