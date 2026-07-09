MHO\_MultitonePhaseCorrection
=============================

Purpose
-------
``MHO_MultitonePhaseCorrection`` applies multi-tone phase calibration (pcal)
corrections to visibility data. It fits a mean phase offset and delay from the
multi-tone pcal calibration tone signals injected at each station, resolves delay
ambiguities, and applies the resulting correction as a complex phasor to the
visibility data. This operator handles both reference and remote stations, with
separate ``ref_multitone_pcal`` and ``rem_multitone_pcal`` instances.

Control File Trigger
--------------------
- **Keywords:** ``ref_multitone_pcal``, ``rem_multitone_pcal``
- **Category:** calibration
- **Priority:** 3.1
- **Internal:** This operator takes no external parameters; it is configured through station parameters in the parameter store (e.g., ``pc_period``, ``sampler_delay_x/y/r/l``, ``pc_tonemask``).

The builder checks the ``pc_mode`` parameter (defaults to ``multitone``).
Station-specific ``pc_mode`` values at ``/control/station/<station_id>/pc_mode``
override the generic setting.

Input Data
----------
This operator acts on the ``visibility_type`` container in-place, and also requires a ``multitone_pcal_type`` container (either ``ref_pcal`` or ``rem_pcal``) and optionally a ``weight_type`` container for per-AP weighting of pcal tone phasors.

Algorithm
---------
**Initialization (``InitializeInPlace``):**

Simply checks that the ``multitone_pcal_type`` pointer is non-null. The operator returns false if no pcal data is available.

**Execution (``ExecuteInPlace``):**

1. Iterate over both stations (reference and remote). For each, call ``IsApplicable`` to check whether the station's Mk4ID or 2-character station code matches the operator's selection criteria.
2. If applicable, compute the time offset between the pcal data start and visibility data start, then call ``InterpolatePCData`` to temporally interpolate pcal tone phasors to align with the visibility accumulation periods. This interpolation mimics the ``ap_mean`` function from the original HOPS3 ``pcal_inter.c``.
3. Trim the pcal data to the visibility time range using ``MHO_PhaseCalibrationTrim``.
4. Retrieve ``pc_tonemask`` channel names and bitmasks from the pcal data (if present).
5. For each polarization in the pcal data, match against the visibility polarization products via ``PolMatch``, then call ``ApplyPCData`` for each matching pol-product.

**Apply PC Data (``ApplyPCData``):**

For each channel:

1. Retrieve the channel's sky frequency, bandwidth, net sideband, and fourfit channel label. Determine the channel's lower and upper frequency limits based on sideband.
2. Call ``DetermineChannelToneIndexes`` to find which pcal tone frequencies fall within the channel's frequency range.
3. Retrieve the sampler delay for this channel using the ``ref_sampler_index`` or ``rem_sampler_index`` label. If no sampler delays are defined, delay fitting and ambiguity resolution are skipped.
4. Determine the tone mask (a 32-bit integer) for this channel. For LSB channels, the bitmask is left-shifted by :math:`32 - n_{\mathrm{tones}}` and bit-reversed.
5. Segment the accumulation periods according to ``fPCPeriod`` (default 1 AP per segment). For each segment:

   a. Sum the weighted pcal tone phasors for all tones in the channel, respecting the tone mask (masked tones get weight 0).
   b. Average the accumulated phasors over the segment.
   c. Call ``FitPCData`` to extract a mean phase, delay, and magnitude from the averaged tones.
6. Store the fitted pcal parameters (segment start/end APs, magnitude, phase, delay) as labels on the channel axis.

**Sampler-Delay Averaging:**

After fitting all channels, the operator averages the fitted pcal delays across all channels sharing the same sampler delay index. This "averaged" delay is stored as ``ref_mtpc_delays_applied_<pol>`` or ``rem_mtpc_delays_applied_<pol>`` on the channel axis.

**Applying Corrections:**

For each channel and segment, apply two corrections. Let :math:`\varphi_{\rm pc}` and :math:`\tau_{\rm pc}` denote the fitted pcal phase and delay from ``FitPCData`` (the ``pc_phase`` and ``pc_delay`` fields), and let :math:`s_b = +1` for LSB, :math:`-1` for USB:

1. **Phase correction:** A single complex phasor applied to the entire (pol-product, channel, AP-range) sub-view:

   .. math::

      \Phi_{\rm pc} = \exp\!\left(-i \cdot \varphi_{\rm pc}\right)

   The phasor is conjugated for the reference station and for USB channels.
2. **Delay correction (if sampler delays are defined):** A frequency-dependent phasor applied per spectral bin:

   .. math::

      \Phi_{\rm delay} = \exp\!\left(-2\pi i \cdot s_b \cdot \tau_{\rm pc} \cdot \Delta f\right)

   where :math:`\Delta f = (f_s - \text{bandwidth}/2) \cdot 10^6` Hz (offset from channel mid-frequency). The delay phasor is also conjugated for the reference station.

**Fit PC Data (``FitPCData``):**

Let :math:`\tau_{\rm station}` and :math:`\tau_{\rm sampler}` denote the ``station_delay`` and ``sampler_delay`` parameters.

1. Compute the delay ambiguity spacing (the ``pc_amb`` value):

   .. math::

      \tau_{\rm amb} = \left| \frac{1}{f_1 - f_0} \right| \cdot 10^{-6}

   (in nanoseconds), where :math:`f_0, f_1` are the first two tone frequencies in MHz.
2. Execute a forward FFT on the averaged tone phasors to find the peak in delay space.
3. Fit a parabola to the peak and its neighbors to obtain sub-sample delay precision.
4. Resolve the delay ambiguity by shifting the delay into the window :math:`[\tau_{\rm station} + \tau_{\rm sampler} \pm \tau_{\rm amb}/2]`. :math:`\tau_{\rm station}` is currently hardcoded to ``0.0`` here, since station delay corrections are applied separately by :hops:`MHO_StationDelayCorrection`.
5. Rotate each tone phasor by the resolved delay :math:`\tau` (zero rotation at channel center frequency) and average to obtain the mean phasor:

   .. math::

      \bar{p} = \sum_{i=0}^{n_{\mathrm{tones}}-1} \left( p_i \cdot \exp\!\left(i \cdot s_b \cdot 2\pi \cdot \tau \cdot \Delta f_i\right) \right)

   where :math:`\Delta f_i = (f_{\mathrm{center}} - f_i) \cdot 10^6` Hz.
6. Multiply the summed mean phasor by :math:`s_b` and conjugate it, :math:`\bar{p} \gets \overline{s_b \cdot \bar{p}}`, then extract magnitude, phase, and delay from the result.

Effect on Data
--------------
The operator multiplies each visibility data segment by a complex phase phasor
derived from the injected multi-tone phase-cal tones, and optionally by a
frequency-dependent delay correction phasor. Both corrections are conjugated
for the reference station (but not the remote), and the phase correction is
additionally conjugated for USB channels. The result is that the residual
instrumental phase and delay errors introduced by the station's receiver chain
between the point of injection and the backend are removed.
