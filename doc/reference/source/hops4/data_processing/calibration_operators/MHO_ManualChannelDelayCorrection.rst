MHO\_ManualChannelDelayCorrection
=================================

Purpose
-------
``MHO_ManualChannelDelayCorrection`` applies user-specified, per-channel delay
corrections to visibility data. For each channel matching a configured label,
the operator computes a frequency-dependent phase correction phasor at every
spectral point within that channel and multiplies the visibility data
accordingly. The delay is specified in nanoseconds and produces a linear
phase ramp across the channel's spectral samples.

Control File Trigger
--------------------
- **Keywords:** ``delay_offs``, ``delay_offs_x``, ``delay_offs_y``, ``delay_offs_r``, ``delay_offs_l``
- **Category:** calibration
- **Priority:** 3.5

The suffix-free keyword (``delay_offs``) applies to all polarizations;
suffixed variants (``_x``, ``_y``, ``_r``, ``_l``) restrict the correction to
the named polarization.

.. list-table:: Parameters for ``delay_offs`` variants
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - channel_names
     - string
     - Comma-separated channel labels to which the correction applies.
   * - pc_delays
     - list_real
     - Delay offset in nanoseconds, one per channel name.

Input Data
----------
This operator acts on the ``visibility_type`` container.

Algorithm
---------
``MHO_ManualChannelDelayCorrection`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

The builder (``MHO_ManualChannelDelayCorrectionBuilder``) parses the polarization from the keyword name, constructs a channel-label-to-delay map from the ``channel_names`` and ``pc_delays`` parameters, and retrieves target station identifiers.

**Execution (``ExecuteInPlace``):**

1. Iterate over the reference (index 0) and remote (index 1) stations.
2. For each station, check applicability via ``IsApplicable`` (same station-identity matching as ``MHO_LSBOffset``).
3. If applicable, retrieve the polarization-product axis, channel axis, and frequency axis.
4. For each polarization product:

   a. Check polarization match via ``PolMatch``: if the operator's polarization is ``?`` (wildcard), all products match; otherwise, the character at the station's index in the pol-product label must equal the configured polarization (case-insensitive).

   b. For each (channel-label, delay) pair in the configured map:

      i. Iterate over all channels in the visibility container. For each channel, retrieve the ``channel_label`` and compare via ``LabelMatch``: if the configured label contains no ``+`` or ``-``, the given label's ``+``/``-`` suffixes (used for DSB halves) are stripped before comparison.

      ii. On label match, retrieve the channel's ``bandwidth`` tag. If absent, log an error and skip.

      iii. Store the delay value (in nanoseconds) as metadata on the channel axis under the key ``ref_delayoff_<pol>`` or ``rem_delayoff_<pol>`` (depending on station index).

      iv. Compute the effective sample period assuming Nyquist sampling:

          .. math::

             t_{\rm eff} = \frac{1}{2 \cdot B \cdot 10^6}

          where :math:`B` is the bandwidth in MHz.

      v. For each spectral point ``sp = 0 .. N_sp-1``:

         (1) Retrieve the frequency offset from the frequency axis: ``Delta f = f(sp) * 10^6`` (in Hz).

         (2) Compute the primary phase term:

             .. math::

                \theta = -2\pi \cdot \Delta f \cdot \tau \cdot 10^{-9}

             where :math:`\tau` is the delay in nanoseconds.

         (3) Compute the geodetic phase-shift correction:

             .. math::

                \phi_{\rm shift} = -\frac{\pi}{2} \cdot \frac{\tau \cdot 10^{-9}}{t_{\rm eff}}

         (4) Scale by a spectral-point-dependent factor:

             .. math::

                \phi_{\rm shift} \gets \phi_{\rm shift} \cdot \frac{-(2 N_{\rm sp} - 2)}{2 N_{\rm sp}}

         (5) Accumulate: ``theta <- theta + phi_shift``.

         (6) Construct the phasor: ``Phi = exp(i * theta)``.

         (7) For the remote station (``st_idx = 1``), conjugate the phasor: ``Phi_rem = conj(Phi)``.

         (8) Multiply the visibility slice at (pol-product, channel, all APs, spectral point) by ``Phi``.

Effect on Data
--------------
For each matching station and polarization, the operator applies a
frequency-dependent phase rotation at every spectral point within each
matching channel. The phase rotation is a linear ramp in frequency determined
by the user-specified delay (in nanoseconds), with an additional geodetic
phase-shift correction term. The reference station receives the phasor
directly; the remote station receives the conjugated phasor. The delay value
is also stored as metadata on the channel axis for later inspection.
