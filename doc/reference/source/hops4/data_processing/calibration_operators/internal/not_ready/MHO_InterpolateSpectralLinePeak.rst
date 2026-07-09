MHO\_InterpolateSpectralLinePeak
================================

Purpose
-------
``MHO_InterpolateSpectralLinePeak`` performs fine sub-bin interpolation around the coarse peak found by ``MHO_SpectralLineFringeSearch`` for spectral-line observations. It applies independent 3-point parabolic interpolation along the delay-rate and intra-channel frequency dimensions to locate the sub-bin amplitude maximum, then extracts the fringe phase and computes the phase delay at the spectral-line frequency.

Control File Trigger
--------------------
This operator is an internal utility with no control file keyword. It is invoked internally by the spectral-line fringe-fitting pipeline after ``MHO_SpectralLineFringeSearch`` identifies the coarse peak location.

Input Data
----------
This operator acts on a ``visibility_type`` container produced by ``MHO_SpectralLineFringeSearch``, with layout ``[1][channel][DR_bin][freq_bin]``. The caller also provides the ``weight_type`` container, the delay-rate axis, the reference frequency (in MHz), and the coarse peak bin indices (channel, DR bin, freq bin).

Algorithm
---------
**Initialization (``Initialize``):**

1. Validate spec\_dr data, weights, DR axis (need :math:`\geq 3` bins).
2. Retrieve ``total_summed_weights`` from weights container; default to 1.0 if missing or non-positive.

**Execution (``Execute``):**

1. **Delay-rate interpolation:** Extract amplitudes at three adjacent DR bins (peak :math:`- 1`, peak, peak :math:`+ 1`), wrapping with modulo arithmetic at the axis boundary. Apply 3-point parabolic interpolation to find the fractional sub-bin offset:

   .. math::

      \delta_\mathrm{DR} = \frac{A_{-1} - A_{+1}}{2\,(A_{-1} - 2A_0 + A_{+1})}

   The offset is clamped to :math:`[-0.5, 0.5]`. If the denominator is near-zero (flat region), the offset is zero. The fine delay rate is:

   .. math::

      \mathrm{DR}_\mathrm{fine} = \mathrm{DR}_\mathrm{peak} + \delta_\mathrm{DR} \cdot \Delta_\mathrm{DR}.

2. **Intra-channel frequency interpolation:** Similarly, extract amplitudes at three adjacent frequency bins and compute the parabolic sub-bin offset :math:`\delta_\mathrm{freq}`. The peak sky frequency (in MHz) is:

   .. math::

      \nu_\mathrm{peak} = \nu_\mathrm{channel} + \bigl( f_\mathrm{peak} + \delta_\mathrm{freq} \cdot \Delta_\mathrm{freq} \bigr)

   where :math:`\nu_\mathrm{channel}` is the channel-axis value and :math:`f_\mathrm{peak}` is the frequency-axis value at the peak bin.

3. **Fringe amplitude and phase:** Read the complex visibility at the coarse peak bin. The fringe amplitude is normalised:

   .. math::

      A_\mathrm{fringe} = \frac{|V_\mathrm{peak}|}{W_\mathrm{total}}

   and the fringe phase is :math:`\phi_\mathrm{fringe} = \arg(V_\mathrm{peak})` in radians.

4. **Fringe rate:**

   .. math::

      f_\mathrm{fringe} = \mathrm{DR}_\mathrm{fine} \cdot (\nu_\mathrm{ref} \times 10^6)

   where :math:`\nu_\mathrm{ref}` is the reference frequency in MHz.

5. **Phase delay:** The phase delay at the spectral-line frequency is:

   .. math::

      \tau_\mathrm{phase} = \frac{\phi_\mathrm{fringe}}{2\pi \, \nu_\mathrm{peak} \cdot 10^6}

   This is the *phase* delay, not the group delay (which is undefined for a spectrally narrow source).

Effect on Data
--------------
This operator does not modify any input data. It produces refined scalar outputs: the peak sky frequency (MHz), fine-interpolated delay rate (s/s), fringe rate (Hz), fringe amplitude (normalised), fringe phase (radians), and phase delay (seconds). These are accessible via ``GetPeakSkyFrequencyMHz``, ``GetDelayRate``, ``GetFringeRate``, ``GetFringeAmplitude``, ``GetFringePhase``, and ``GetPhaseDelay``.
