MHO\_ManualChannelPhaseCorrection
=================================

Purpose
-------
``MHO_ManualChannelPhaseCorrection`` applies user-specified, per-channel phase
corrections to visibility data. For each channel matching a configured label,
the operator constructs a constant phase phasor and multiplies all visibility
data within that channel (across all polarization products matching the
configured polarization, all spectral points, and all accumulation periods).
The operator handles conjugation based on both the channel's sideband
(lower vs.\ upper) and the station index (reference vs.\ remote).

Control File Trigger
--------------------
- **Keywords:** ``pc_phases``, ``pc_phases_x``, ``pc_phases_y``, ``pc_phases_r``, ``pc_phases_l``
- **Category:** calibration
- **Priority:** 3.5

The suffix-free keyword (``pc_phases``) applies to all polarizations; suffixed variants (``_x``, ``_y``, ``_r``, ``_l``) restrict the correction to the named polarization.

.. list-table:: Parameters for ``pc_phases`` variants
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - channel_names
     - string
     - Comma-separated channel labels to which the correction applies.
   * - pc_phases
     - list_real
     - Phase correction in degrees, one per channel name.

Input Data
----------
This operator acts on the ``visibility_type`` container.

Algorithm
---------
``MHO_ManualChannelPhaseCorrection`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

The builder (``MHO_ManualChannelPhaseCorrectionBuilder``) parses the polarization from the keyword name, constructs a channel-label-to-phase map from the ``channel_names`` and ``pc_phases`` parameters, and retrieves target station identifiers.

**Execution (``ExecuteInPlace``):**

1. Iterate over the reference (index 0) and remote (index 1) stations.
2. For each station, check applicability via ``IsApplicable`` (same station-identity matching as ``MHO_LSBOffset``).
3. If applicable, retrieve the polarization-product axis and channel axis.
4. For each polarization product:

   a. Check polarization match via ``PolMatch``: if the operator's polarization is ``?`` (wildcard), all products match; otherwise, the character at the station's index in the pol-product label must equal the configured polarization (case-insensitive).

   b. Determine a metadata key for storing the applied phase: ``ref_pcphase_<pol>`` for the reference station, ``rem_pcphase_<pol>`` for the remote station.

   c. For each (channel-label, phase) pair in the configured map:

      i. Iterate over all channels in the visibility container. For each channel, retrieve the ``channel_label`` and compare via ``LabelMatch``: if the configured label contains no ``+`` or ``-``, the given label's ``+``/``-`` suffixes (used for DSB halves) are stripped before comparison.

      ii. On label match, retrieve the channel's ``net_sideband`` label (if present).

      iii. Construct the base phasor from the phase value in degrees:

          .. math::

             \Phi = \exp\!\left(i \cdot \phi_{\rm pc} \cdot \pi/180\right)

      iv. Apply conjugation rules:

          - If ``net_sideband`` equals ``L`` (lower sideband), conjugate: ``Phi = conj(Phi)``.
          - If the station is the reference station (``st_idx = 0``), conjugate: ``Phi = conj(Phi)``.

      v. Multiply the visibility sub-view for (pol-product, channel, all APs, all spectral points) by ``Phi``.

      vi. Store the phase value (converted to radians) as metadata on the channel axis under the ``ref_pcphase_<pol>`` or ``rem_pcphase_<pol>`` key.

Effect on Data
--------------
For each matching station and polarization, the operator multiplies all
visibility data within each matching channel (across all spectral points
and accumulation periods) by a constant complex phasor
``exp(i * phi_pc * pi/180)``. The phasor is conjugated for lower-sideband
channels and for the reference station. The phase value (in radians) is stored
as metadata on the channel axis for later inspection. The correction is
independent of frequency and time within the channel.
