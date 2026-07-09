MHO_NormFX (Base Class)
========================

Purpose
-------
Abstract base class defining the interface for normalized cross-spectrum
(NormFX) operators. NormFX performs the frequency-to-lag transform that
converts correlator visibilities into a form suitable for wideband
fringe fitting. The base class is a template
for ``MHO_UnaryOperator<visibility_type>`` that accepts
a ``weight_type`` for weighting during the transform. Concrete
implementations handle different sideband configurations.

Control File Trigger
--------------------
This operator is internal and not directly triggerable from the control file.
It is the base class for:

- ``MHO_SingleSidebandNormFX`` -- used when all channels share the same sideband
- ``MHO_MixedSidebandNormFX`` -- used when data contains a mix of USB, LSB, and DSB channels

Input Data
----------
- ``visibility_type`` -- primary input (4D tensor: polprod $\times$ channel $\times$ AP $\times$ freq)
- ``weight_type`` -- associated weights (set via ``SetWeights()``)

Algorithm
---------
The ``MHO_NormFX`` base class defines four pure virtual functions that concrete subclasses must implement:

- ``InitializeInPlace(visibility_type* in)`` -- prepare for in-place transform
- ``InitializeOutOfPlace(const visibility_type* in, visibility_type* out)`` -- copy input to output, then prepare
- ``ExecuteInPlace(visibility_type* in)`` -- perform the transform
- ``ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)`` -- transform input into output

The core algorithm shared by all implementations:

1. **Zero-padding:** Extend the frequency axis by a factor of 4x (single-sideband) or 8x (mixed sideband) to provide interpolation resolution in the delay domain.
2. **Weighting:** Multiply each visibility element by its corresponding weight (or square root of weight, depending on convention).
3. **Forward FFT:** Apply FFT along the (padded) frequency axis, converting from frequency to lag (delay) domain.
4. **Cyclic rotation:** Shift the FFT output so the zero-delay point appears at the expected array index (an ``fftshift`` operation).
5. **LSB conjugation:** For lower-sideband channels, conjugate the output to account for the inverted frequency axis.
6. **Normalization:** Divide by the total weight or number of contributing spectral samples.

The mathematical transform is:

.. math::
   S(\tau) = \frac{1}{W} \sum_{f} V(f) \cdot w(f) \cdot \exp(2\pi i f \tau)

where $\tau$ is the lag (delay) corresponding to the FFT output bins.

Effect on Data
--------------
The NormFX transform converts the visibility from the frequency domain to
the lag (delay) domain. The output frequency axis represents delay samples
(nanoseconds) rather than frequency channels. The transform preserves the
polprod, channel, and AP axes. For single-sideband data, the output frequency
axis has size $\approx 4 \times N_{\mathrm{freq}}$.
