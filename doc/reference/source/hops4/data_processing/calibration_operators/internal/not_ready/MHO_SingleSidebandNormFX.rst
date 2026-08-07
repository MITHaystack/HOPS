MHO\_SingleSidebandNormFX
=========================

Purpose
-------
``MHO_SingleSidebandNormFX`` transforms visibility data from the frequency
domain into the single-band delay (SBD) lag domain for datasets where all
channels share a single sideband type (all USB or all LSB, but not a mixture).
It implements the core frequency-to-lag transform with a reduced zero-padding
factor of ``P = 4`` (compared to ``P = 8`` in the mixed-sideband variant,
since only one sideband needs to be handled).

Control File Trigger
--------------------
This operator is internal and is not directly triggered by a control-file keyword.
It is instantiated by the fringe-fitting pipeline when the input data contains
only single-sideband channels. For a full description of the frequency-to-lag
transform, see the broadband fringe-fitting algorithm description.

Input Data
----------
This operator acts on the ``visibility_type`` container. It produces output
in a separate ``visibility_type`` container (out-of-place operation), though
in-place wrappers are provided that copy the result back.

Algorithm
---------
**Initialization (``InitializeOutOfPlace``):**

1. Use ``MHO_SBDTableGenerator`` to set up the output container with the FREQ_AXIS expanded by a factor of 4:

   .. math::

      \text{out\_dims}[\text{FREQ\_AXIS}] = 4 \cdot N_s

   where :math:`N_s` is the number of frequency sub-bins. All other axes (POLPROD_AXIS, CHANNEL_AXIS, TIME_AXIS) are copied unchanged.
2. Initialize the ``fZeroPadder`` to copy input data into the output container with 4x end-padding along FREQ_AXIS.
3. Initialize ``fFFTEngine`` for a forward 1-D DFT along FREQ_AXIS.
4. Initialize ``fCyclicRotator`` with offset :math:`\text{out\_dims}[\text{FREQ\_AXIS}] / 2 = 2 \cdot N_s` to center zero delay.
5. Pre-cache the indices of all LSB channels by inspecting the ``net_sideband`` label on the channel axis. This avoids repeated metadata lookups during execution.

**Execution (``ExecuteOutOfPlace``):**

1. **Zero-Padding:** Execute ``fZeroPadder`` to copy the input visibility data into the output container, padding the FREQ_AXIS with zeros at the end:

   .. math::

      \mathcal{S}[p,c,a,s] = \begin{cases} V[p,c,a,s], & s = 0, \dots, N_s - 1 \\ 0, & s = N_s, \dots, 4N_s - 1 \end{cases}

2. **Forward FFT:** Execute ``fFFTEngine`` along FREQ_AXIS:

   .. math::

      \mathcal{S}[p,c,a,\ell] = \sum_{s=0}^{N_s-1} V[p,c,a,s] \; \exp\!\left(-\frac{2\pi i \, \ell \, s}{4 N_s}\right) \qquad \ell = 0,\dots,4N_s-1

3. **Cyclic Rotation:** Execute ``fCyclicRotator`` with offset :math:`2 \cdot N_s` to center the zero-delay peak:

   .. math::

      \mathcal{S}'[\ell] = \mathcal{S}\bigl[(\ell + 2N_s) \bmod (4N_s)\bigr]

   After rotation, negative delays occupy indices :math:`[0, 2N_s)` and positive delays occupy indices :math:`[2N_s, 4N_s)`.
4. **LSB Conjugation:** For each channel in the pre-cached LSB index list, complex-conjugate the entire SBD slice:

   .. math::

      \mathcal{S}'[p,\,c_{\mathrm{LSB}},\,a,\,\ell] \leftarrow \overline{\mathcal{S}'[p,\,c_{\mathrm{LSB}},\,a,\,\ell]}

   This is equivalent to flipping the frequency axis before the FFT, ensuring LSB and USB channels share a common delay-space reference.
5. **Normalization:** Divide the entire array by the original (unpadded) number of frequency sub-bins:

   .. math::

      \mathcal{S}'[p,c,a,\ell] \leftarrow \frac{\mathcal{S}'[p,c,a,\ell]}{N_s}

Effect on Data
--------------
The output is a visibility container whose FREQ_AXIS now represents single-band
delay (lag) rather than frequency sub-bin. The lag axis has ``4 * N_s`` elements
centered around zero delay. For LSB channels, the SBD slice is
complex-conjugated after the FFT to correct for the inverted frequency axis.
