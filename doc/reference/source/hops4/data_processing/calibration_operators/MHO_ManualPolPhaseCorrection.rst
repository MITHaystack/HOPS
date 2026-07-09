MHO\_ManualPolPhaseCorrection
=============================

Purpose
-------
``MHO_ManualPolPhaseCorrection`` applies a user-specified polarization-specific
phase offset to visibility data. The operator computes a constant phase
correction phasor from the offset (in degrees) and multiplies all matching
visibility elements per channel. The correction accounts for sideband
inversion (LSB vs USB) and distinguishes between reference and remote stations.

Control File Trigger
--------------------
- **Keywords:** ``pc_phase_offset_x``, ``pc_phase_offset_y``, ``pc_phase_offset_r``, ``pc_phase_offset_l``
- **Category:** calibration
- **Priority:** 3.5

Each keyword selects one of the four polarization states (X, Y, R, L).
The keyword name determines the polarization via the suffix (``_x``, ``_y``, ``_r``, ``_l``).

.. list-table:: Parameters for ``pc_phase_offset_<pol>`` keywords
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - value
     - real
     - Phase offset in degrees applied to the specified polarization.

Input Data
----------
This operator acts on the ``visibility_type`` container.

Algorithm
---------
``MHO_ManualPolPhaseCorrection`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

The builder (``MHO_ManualPolPhaseCorrectionBuilder``) parses the polarization from the keyword name and resolves station identifiers.

**Execution (``ExecuteInPlace``):**

1. Iterate over the reference (index 0) and remote (index 1) stations.
2. For each station, check applicability via ``IsApplicable``: the operator matches if any configured station identity equals the station's Mark4 ID (1-character match) or 2-character station code (2-character match). Wildcard ``?`` matches any Mark4 ID, and ``??`` matches any station code.
3. If applicable, retrieve the polarization-product axis and channel axis.
4. For each polarization product:

   a. Check polarization match via ``PolMatch``: the character at the station's index in the pol-product label must equal the configured polarization (case-insensitive).

   b. Store the phase offset (converted to radians) as metadata on the pol-product axis under the key ``ref_pcphase_offset_<pol>`` or ``rem_pcphase_offset_<pol>`` (depending on station index).

   c. For each channel ``ch``:

      (1) Convert the phase offset from degrees to radians: ``theta = phase_offset * pi / 180``.

      (2) Construct the correction phasor: ``Phi = exp(i * theta)``.

      (3) Apply sideband conjugation: if the channel's ``net_sideband`` tag equals ``L`` (lower sideband), conjugate the phasor: ``Phi = conj(Phi)``.

      (4) Apply station conjugation: if the station index is 0 (reference station), conjugate the phasor: ``Phi = conj(Phi)``.

      (5) Multiply the visibility sub-view at (pol-product, channel) by ``Phi``.

Effect on Data
--------------
For each matching station and polarization, the operator applies a constant
phase rotation to every channel in the visibility data. The phase rotation is
determined by the user-specified offset in degrees. The correction phasor is
conjugated for both lower-sideband data and reference-station data (so the net
sign depends on which combination applies). The phase offset value (in radians)
is stored as metadata on the polarization-product axis for later inspection.
