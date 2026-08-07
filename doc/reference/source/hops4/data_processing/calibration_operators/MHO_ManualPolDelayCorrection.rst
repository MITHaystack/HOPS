MHO\_ManualPolDelayCorrection
=============================

Purpose
-------
``MHO_ManualPolDelayCorrection`` applies a user-specified polarization-specific
delay offset to visibility data. The operator computes a frequency-dependent
phase correction phasor from the delay (in nanoseconds) and multiplies all
matching visibility elements. The correction is applied independently to each
channel, accounts for sideband inversion (LSB vs USB), and distinguishes between
reference and remote stations.

Control File Trigger
--------------------
- **Keywords:** ``pc_delay_x``, ``pc_delay_y``, ``pc_delay_r``, ``pc_delay_l``
- **Category:** calibration
- **Priority:** 3.5

Each keyword selects one of the four polarization states (X, Y, R, L).
The keyword name determines the polarization via the suffix (``_x``, ``_y``, ``_r``, ``_l``).

.. list-table:: Parameters for ``pc_delay_<pol>`` keywords
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - value
     - real
     - Delay offset in nanoseconds applied to the specified polarization.

Input Data
----------
This operator acts on the ``visibility_type`` container.

Algorithm
---------
``MHO_ManualPolDelayCorrection`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

The builder (``MHO_ManualPolDelayCorrectionBuilder``) parses the polarization from the keyword name, retrieves the reference frequency from the parameter store (``/control/config/ref_freq``), and resolves station identifiers.

**Execution (``ExecuteInPlace``):**

1. Iterate over the reference (index 0) and remote (index 1) stations.
2. For each station, check applicability via ``IsApplicable``: the operator matches if any configured station identity equals the station's Mark4 ID (1-character match) or 2-character station code (2-character match). Wildcard ``?`` matches any Mark4 ID, and ``??`` matches any station code.
3. If applicable, retrieve the polarization-product axis and channel axis.
4. For each polarization product:

   a. Check polarization match via ``PolMatch``: the character at the station's index in the pol-product label must equal the configured polarization (case-insensitive).

   b. Compute the delay in seconds: ``tau = delay_offset * 10^-9``.

   c. Store the delay offset (in nanoseconds) as metadata on the pol-product axis under the key ``ref_delayoff_<pol>`` or ``rem_delayoff_<pol>`` (depending on station index).

   d. For each channel ``ch``:

      (1) Retrieve the channel frequency from the channel axis: ``nu_ch`` (in MHz).

      (2) Compute the frequency offset from the reference frequency:

          .. math::

             \Delta f = (\nu_{\rm ch} - \nu_{\rm ref}) \cdot 10^6

          where :math:`\nu_{\rm ref}` is the reference frequency in MHz and :math:`\Delta f` is in Hz.

      (3) Compute the phase angle:

          .. math::

             \theta = 2\pi \cdot \Delta f \cdot \tau

      (4) Construct the correction phasor: ``Phi = exp(i * theta)``.

      (5) Apply sideband conjugation: if the channel's ``net_sideband`` tag equals ``L`` (lower sideband), conjugate the phasor: ``Phi = conj(Phi)``.

      (6) Apply station conjugation: if the station index is 0 (reference station), conjugate the phasor: ``Phi = conj(Phi)``.

      (7) Multiply the visibility sub-view at (pol-product, channel) by ``Phi``.

Effect on Data
--------------
For each matching station and polarization, the operator applies a
frequency-dependent phase rotation to every channel in the visibility data.
The phase rotation encodes a linear delay offset in nanoseconds relative to
the user-configured reference frequency. The correction phasor is conjugated
for both lower-sideband data and reference-station data (so the net sign depends
on which combination applies). The delay offset value is stored as metadata on
the polarization-product axis for later inspection.
