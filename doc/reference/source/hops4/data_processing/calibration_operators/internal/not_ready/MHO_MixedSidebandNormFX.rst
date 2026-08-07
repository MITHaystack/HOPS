MHO\_MixedSidebandNormFX
========================

Purpose
-------
``MHO_MixedSidebandNormFX`` transforms visibility data from the frequency
domain into the single-band delay (SBD) lag domain for datasets containing a
mixture of upper sideband (USB), lower sideband (LSB), and optionally
double-sideband (DSB) channels. It is a superset of
the ``MHO_SingleSidebandNormFX`` algorithm, adding special
handling for LSB conjugation during the workspace fill step and
weighted merging of DSB channel pairs.

Control File Trigger
--------------------
This operator is internal and is not directly triggered by a control-file
keyword. It is instantiated by the fringe-fitting pipeline when the input
data contains a mixture of USB and LSB sidebands or when DSB channel pairs
are detected. For a full description of the frequency-to-lag transform, see
:doc:`the broadband fringe-fitting algorithm reference </hops4/data_processing/algorithm_and_output/algorithm>`.

Input Data
----------
This operator acts on the ``visibility_type`` container. It produces output
in a separate ``visibility_type`` container (out-of-place operation), though
in-place wrappers are provided that copy the result back.

Algorithm
---------
The algorithm uses an 8x zero-padding factor followed by 2x sub-sampling, which
yields an effective padding of 4x, matching the output dimension
of ``MHO_SingleSidebandNormFX``.

**Initialization (``InitializeOutOfPlace``):**

1. Inspect the channel axis for ``net_sideband`` labels to classify channels as USB (``U``), LSB (``L``), or double-sideband (via the ``double_sideband`` interval label). Log warnings for mixed USB/LSB data and DSB channels, as support for these configurations is experimental.
2. Allocate the output container with the FREQ_AXIS expanded by a factor of 4:

   .. math::

      \text{out\_dims}[\text{FREQ\_AXIS}] = 4 \cdot N_s

   where :math:`N_s` is the number of frequency sub-bins. Copy all non-frequency axes (POLPROD_AXIS, CHANNEL_AXIS, TIME_AXIS) unchanged.
3. Create a temporary workspace with the FREQ_AXIS expanded by a factor of 8:

   .. math::

      \text{work\_dims}[\text{FREQ\_AXIS}] = 8 \cdot N_s

4. Initialize sub-operators:

   - ``fZeroPadder``: pads the input to 8x along FREQ_AXIS (end-padded).
   - ``fNaNBroadcaster``: masks NaN values in the workspace.
   - ``fFFTEngine``: performs a forward 1-D DFT along FREQ_AXIS.
   - ``fSubSampler``: sub-samples the FFT output by a stride of 2 along FREQ_AXIS, yielding :math:`4 \cdot N_s` output lags.
   - ``fCyclicRotator``: cyclically shifts the output by :math:`2 \cdot N_s` positions along FREQ_AXIS to center zero delay.

**Execution (``ExecuteOutOfPlace``):**

1. **Fill Workspace:** Call ``FillWorkspace``, which executes the zero-padder and then performs a sideband-aware copy from input to workspace:

   - For USB channels, copy directly:

     .. math::

        W[p, ch, a, s] += V[p, ch, a, s]

   - For LSB channels, complex-conjugate and reverse the frequency axis with an offset of ``lsb_shift = work_size / 2``:

     .. math::

        W[p, ch, a, \text{lsb\_shift} - s] += \overline{V[p, ch, a, s]}

   This placement ensures that LSB and USB data occupy a common frequency-to-delay reference frame in the workspace.
2. **Treat DSB Channels:** Call ``TreatDoubleSidebandChannels``. For each DSB channel pair identified by the ``dsb_partner`` label, compute a weighted average of the LSB and USB workspace values:

   .. math::

      W_{\mathrm{merged}} = \frac{w_{\mathrm{LSB}} \cdot W_{\mathrm{LSB}} + w_{\mathrm{USB}} \cdot W_{\mathrm{USB}}}{w_{\mathrm{LSB}} + w_{\mathrm{USB}}}

   and write the merged value to both the LSB and USB entries. If no weights are available, equal weighting is assumed.
3. **NaN Masking:** Execute ``fNaNBroadcaster`` to propagate NaN masks.
4. **Forward FFT:** Execute ``fFFTEngine`` along FREQ_AXIS:

   .. math::

      \mathcal{F}[\ell] = \sum_{s=0}^{8N_s-1} W[s] \; \exp\!\left(-\frac{2\pi i \, \ell \, s}{8 N_s}\right)

5. **Sub-sampling:** Execute ``fSubSampler`` with stride 2, retaining every second lag to produce :math:`4 \cdot N_s` output points.
6. **Cyclic Rotation:** Execute ``fCyclicRotator`` with offset :math:`2 \cdot N_s` to center the zero-delay peak.
7. **Normalization:** Divide the entire output array by the original (unpadded) number of frequency sub-bins:

   .. math::

      \mathcal{S}'[p,c,a,\ell] \leftarrow \frac{\mathcal{S}'[p,c,a,\ell]}{N_s}

Effect on Data
--------------
The output is a visibility container whose FREQ_AXIS now represents single-band
delay (lag) rather than frequency sub-bin. The lag axis
has ``4 * N_s`` elements, centered around zero delay. LSB channels are 
conjugated and reversed before the FFT so that all sidebands contribute
coherently in delay space. DSB channel pairs are merged by weighted
averaging before the transform.
