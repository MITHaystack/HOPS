MHO\_InterpolateFringePeak
==========================

Purpose
-------
``MHO_InterpolateFringePeak`` performs fine sub-bin interpolation around a
coarse fringe-search peak in three dimensions (single-band delay (SBD),
multi-band delay (MBD), and delay rate (DR) ). It builds
a :math:`5 \times 5 \times 5` cube of counter-rotated fringe
amplitudes about the coarse peak, then uses iterative 5-point Lagrange
interpolation to locate the sub-bin maximum with high precision.
This is the non-optimized (reference) version of the fringe-peak interpolator,
superseded by ``MHO_InterpolateFringePeakOptimized``.

Control File Trigger
--------------------
This operator is an internal utility with no control file keyword.
It is used by the fringe fitter to refine coarse peak locations before
``MHO_InterpolateFringePeakOptimized`` was introduced.

Input Data
----------
This operator acts on the ``visibility_type`` container (the SBD array), the ``weight_type`` container, and external axis data for multi-band delay and delay rate. The caller provides:

- A pointer to the SBD visibility array via ``SetSBDArray``.
- A pointer to the weights array via ``SetWeights``.
- The MBD axis via ``SetMBDAxis`` and the DR axis via ``SetDRAxis``.
- The coarse peak bin indices (SBD, MBD, DR) via ``SetMaxBins``.
- The reference frequency and reference time offset via ``SetReferenceFrequency`` and ``SetReferenceTimeOffset``.

Algorithm
---------
**Initialization (``Initialize``):**

1. Verify the SBD array, weights array, MBD axis, and DR axis are all non-null and non-trivial (size :math:`> 1`).
2. Retrieve the ``total_summed_weights`` scalar from the weights container for amplitude normalization.

**Execution (``Execute`` -> ``fine_peak_interpolation``):**

1. Extract the channel axis (frequency), accumulation-period axis (time), and SBD axis (delay) from the visibility container.
2. Compute bin spacings :math:`\Delta_\mathrm{SBD}`, :math:`\Delta_\mathrm{MBD}`, :math:`\Delta_\mathrm{DR}` from axis deltas.
3. Build a :math:`5 \times 5 \times 5` cube of fringe amplitudes. For each tabular point :math:`(i_\mathrm{SBD}, i_\mathrm{MBD}, i_\mathrm{DR})`:

   a. Compute the actual axis coordinate, wrapping around the peak bin with modulo arithmetic:

      .. math::

         \mathrm{bin}_\mathrm{SBD} = (\mathrm{peak}_\mathrm{SBD} + i_\mathrm{SBD} - 2) \bmod N_\mathrm{SBD}

      (similarly for MBD and DR).

   b. Compute sub-bin MBD and DR coordinates at half-grid spacing around the peak:

      .. math::

         \mathrm{mbd} = \mathrm{MBD}_\mathrm{peak} + 0.5 \cdot (i_\mathrm{MBD} - 2) \cdot \Delta_\mathrm{MBD}

      .. math::

         \mathrm{dr} = \mathrm{DR}_\mathrm{peak} + 0.5 \cdot (i_\mathrm{DR} - 2) \cdot \Delta_\mathrm{DR}

   c. Counter-rotate all visibility data across channels and accumulation periods using ``MHO_FringeRotation::vrot`` with the (dr, mbd) trial values. For each channel :math:`ch` and AP :math:`ap`:

      .. math::

         z \;+=\; V(ch, ap, \mathrm{sbd}_\mathrm{bin}) \cdot v_\mathrm{rot}(\Delta t, \nu_{ch}, \nu_\mathrm{ref}, \mathrm{dr}, \mathrm{mbd}) \cdot w(ch, ap)

      where :math:`\Delta t = t_{ap} + \Delta t_\mathrm{AP}/2 - t_\mathrm{ref}`.

   d. Store :math:`|z| / W_\mathrm{total}` in the cube.

4. Search the cube for the sub-bin maximum using ``max555``, which performs iterative refinement on an :math:`11 \times 11 \times 11` grid using 5-point Lagrange interpolation (``interp555``). The search starts at the cube center and repeatedly reduces the search step until all dimensions converge below :math:`\varepsilon = 10^{-4}`.
5. The Lagrange interpolation follows Abramowitz & Stegun 25.2.15. For a fractional coordinate :math:`p`, the weights are:

   .. math::

      \begin{aligned}
      a_0(p) &= \frac{(p^2-1)p(p-2)}{24}, \\
      a_1(p) &= -\frac{(p-1)p(p^2-4)}{6}, \\
      a_2(p) &= \frac{(p^2-1)(p^2-4)}{4}, \\
      a_3(p) &= -\frac{(p+1)p(p^2-4)}{6}, \\
      a_4(p) &= \frac{(p^2-1)p(p+2)}{24}.
      \end{aligned}

   The interpolated value is the tensor product:

   .. math::

      f(x_0, x_1, x_2) = \sum_{i,j,k} a_i(x_0)\, a_j(x_1)\, a_k(x_2) \, \mathrm{drf}(i,j,k).

6. Convert the fractional peak location :math:`\mathbf{x}_i` back to physical units:

   .. math::

      \Delta_\mathrm{SBD}^\mathrm{fine} &= x_0 \cdot \Delta_\mathrm{SBD} \\
      \Delta_\mathrm{MBD}^\mathrm{fine} &= x_1 \cdot 0.5 \cdot \Delta_\mathrm{MBD} \\
      \Delta_\mathrm{DR}^\mathrm{fine} &= x_2 \cdot 0.5 \cdot \Delta_\mathrm{DR}

7. Output the refined SBD delay, MBD delay, delay rate, fringe rate (:math:`\mathrm{DR} \times \nu_\mathrm{ref}`), and fringe amplitude.

Effect on Data
--------------
This operator does not modify any input data. It produces refined scalar
outputs: the fine-interpolated single-band delay, multi-band delay,
delay rate, fringe rate, and fringe amplitude. These can be accessed via
``GetSBDelay``, ``GetMBDelay``, ``GetDelayRate``, ``GetFringeRate``, and ``GetFringeAmplitude``.
