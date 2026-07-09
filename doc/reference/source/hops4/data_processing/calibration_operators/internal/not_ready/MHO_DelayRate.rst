MHO\_DelayRate
==============

Purpose
-------
``MHO_DelayRate`` performs a time-axis FFT on zero-padded visibility data to
transform accumulation-period (AP) time series into the delay-rate domain.
This operator is used in the ``MBDelaySearch`` stage of the fringe-fitting
pipeline.

Control File Trigger
--------------------
This operator is internal and not directly triggerable from the control file.
It is instantiated and used by ``MBDelaySearch`` for the time-axis FFT that
converts the AP time axis into a delay-rate search axis.

Input Data
----------
This is a binary operator acting on ``visibility_type`` (input 1) and ``weight_type`` (input 2) containers, with output written to an ``sbd_type`` container. The input visibility has four dimensions: polarization product, channel, accumulation period (time), and single-band delay (frequency).

Algorithm
---------
``MHO_DelayRate`` performs the following pipeline: zero-padding, weighting, FFT, and interpolation.

**Initialization (``InitializeImpl``):**

1. Retrieve input dimensions and copy tags from the visibility to the output container.
2. Calculate the delay-rate search-space size: ``N_drsp = 2 * NextLowestPowerOfTwo(N_time)``. The padded FFT size is ``N_p = 4 * N_drsp``.
3. Precompute a per-(channel, delay-rate) interpolation table. For each channel ``ch`` and delay-rate bin ``dr``:

   a. Compute ``b = (f_chan / f_ref) * N_p / N_drsp``.
   b. Compute ``num = (dr - N_drsp/2) * b + 1.5 * N_p``.
   c. Determine linear interpolation indices ``l0 = floor(num % N_p)`` and ``l1 = l0 + 1`` (clamped to ``[0, N_p - 1]``), and weight ``w = (num % N_p) - l0``.
   d. Store ``{l0, l1, w}`` in ``fInterpTable``.
   e. Build ``fPreRotatedInterpTable`` by shifting ``l0, l1`` forward by ``N_p/2`` (modulo ``N_p``), allowing interpolation to read directly from the pre-rotation FFT output.

4. Resize the output container's time axis to ``N_p`` and initialize the zero-padder, FFT engine, and cyclic rotator sub-operators.

**Execution (``ExecuteImpl``):**

The default execution uses ``ExecuteImplOptimized``:

1. **Zero-pad:** Append zeros to the time axis so its length becomes ``N_p``.
2. **Apply data weights:** Multiply each element ``(pp, ch, ap, sbd=0)`` by the corresponding weight from the ``weight_type`` container, iterating only over the original (unpadded) AP range.
3. **FFT:** Perform a forward DFT along the padded time axis. The result has ``N_p`` delay-rate bins.
4. **Interpolation:** Using the pre-rotated interpolation table, for each (pol-product, channel, delay-rate) triple, read two values from the post-FFT array at the precomputed ``l0, l1`` indices and linearly interpolate: ``V(pp, ch, dr, sbd) = V(pp, ch, l0, sbd) * (1-w) + V(pp, ch, l1, sbd) * w``. Results are staged in a workspace buffer to avoid aliasing, then copied back.
5. **Write axis labels:** Set the time-axis labels to represent delay-rate values: ``dr[i] = (i - N_drsp/2) / (delta_t * N_drsp)``, where ``delta_t`` is the input AP time spacing.

Effect on Data
--------------
The operator transforms the input visibility from the time domain to the
delay-rate domain. The output ``sbd_type`` container has the same polarization
product and channel dimensions, but the time axis is replaced by a delay-rate
axis of size ``N_drsp``. The single-band delay (frequency) axis is preserved.
The data values are the magnitude and phase of the frequency-domain
representation, interpolated to a channel-dependent spacing proportional
to the ratio ``f_chan / f_ref``.
