Broadband Fringe-Fitting Algorithm
==================================

This section describes the core single-baseline, single-polarization-product
broadband (bandwidth synthesis) fringe-fitting algorithm as implemented in HOPS4 (ported from HOPS3).
This algorithm performs a three-dimensional search over single-band delay (SBD),
multi-band delay (MBD), and delay rate (DR) to determine the residual geometric
delay parameters of a VLBI observation after initial correlation. The search
employs a coarse FFT-accelerated grid search followed by a fine sub-bin
interpolation step to achieve sub-sample precision. The program which implements
this algorithm (and other associated calibration operations) is called ``fourfit``,
or specifically in HOPS4 (``fourfit4``).

Introduction
------------

In Very Long Baseline Interferometry (VLBI) each radio telescope (a station) in the array records
time-tagged samples of a noise-dominated voltage signal collected from a common
astronomical source. The resulting signals from each station are
then cross-correlated with an appropriately tuned (a priori) delay model in order
to compute the complex visibilities of the source over each baseline (station pair). The first station
in a baseline pair, \A, is referred to as the `reference` station, while the second station, \B, is know as the `remote`
station [#footnote1]_. The resulting visibility data contains residual geometric
phase terms due to imperfect knowledge of the source position, station
coordinates, clock offsets, and atmosphere. *Fringe fitting* is the process of searching
for the set of residual corrections to the delay model parameters that maximize the fringe
amplitude which is produced from the coherent combination of the visibility data across
all spectral channels and time intervals of an observation.

In HOPS4, ``fourfit4`` implements a *broadband* (or *bandwidth-synthesis*) fringe-fitting
algorithm that operates on the full multi-channel visibility data. This algorithm was first described in [Rogers1970]_.
This broadband fringe fitting process simultaneously resolves two main residual corrections to the delay model for
a particular baseline. These are the *multi-band delay* (MBD, otherwise known as the group-delay), and the
delay rate (DR, the time derivative of the group-delay).

In HOPS4 this algorithm is implemented across several source files:

- ``Fringe/MHO_BasicFringeFitter.cc`` -- main orchestration and pipeline control
- ``Fringe/MHO_IonosphericFringeFitter.cc`` -- extended fitter with dTEC correction/search
- ``Calibration/MHO_SingleSidebandNormFX.cc`` -- normalization and frequency-to-lag (SBD) transform (for USB or LSB only channels)
- ``Calibration/MHO_MixedSidebandNormFX.cc`` -- normalization and frequency-to-lag (SBD) transform (for mixed LSB/USB/DSB channels)
- ``Calibration/MHO_MBDelaySearch.cc`` -- coarse three-dimensional grid search over (SBD, MBD, DR)
- ``Calibration/MHO_InterpolateFringePeakOptimized.cc`` -- fine sub-bin fringe peak interpolation
- ``Fringe/MHO_ComputePlotData.cc`` -- post-solution diagnostics and visualization quantities

Input Data Model
----------------

In this document we will restrict the discussion to the visibility data, but for a complete specification of the data
structures used by the fringe-fitting algorithm and HOPS4 see:
:doc:`Data Object Specification </hops4/libraries/containers/Objects>`.

Visibility Tensor
~~~~~~~~~~~~~~~~~

The fundamental input is a four-dimensional tensor of complex visibilities, :math:`\mathbf{V}`,
associated with each baseline:

.. math:: V[p,\,c,\,a,\,s] \in \mathbb{C}

where the axes are:

- **p** POLPROD_AXIS: polarization product index (XX, XY, RR, LR, etc.).
- **c** CHANNEL_AXIS: channel index, this is the coarse frequency subdivision of the data. Each channel carries metadata:
  sky frequency :math:`\nu_c` (MHz), net sideband (USB, LSB, or DSB), and bandwidth
  :math:`B_c` (MHz).
- **a** TIME_AXIS (or accumulation period (AP)): time/AP index, each AP spans a
  fixed integration time :math:`\Delta t` (typically 1 s). The units of this axis are time (sec).
- **s** FREQ_AXIS: frequency spectral-bin (or spectral point) within each channel (typically there are
  64-512 bins, depending on the setup at the time of correlation). Units are MHz.

.. note:: The fringe-fitting algorithm operates on only a single polarization product
  (or linear combination of polarization products) at one time, and after user selection, the
  index *p* is essentially fixed with `p=p'=0`.

Weight Tensor
~~~~~~~~~~~~~

Each visibility is accompanied by a non-negative real weights tensor, :math:`\mathbf{W}`:

.. math:: W[p,\,c,\,a,\,s] \in \mathbb{R}_{\geq 0}

The weights encode the data quality (e.g., zeroed for invalid/missing data) reported by the correlator.
However, the values of the weights may be further modified by additional data operations (i.e. flagging)
before fringe fitting.

.. note:: In ``fourfit4`` the size along the 4th (FREQ_AXIS), *s*, dimension is always set to 1,
  ``fourfit4`` does not yet support intra-channel weights for individual spectral points.

In order to form weighted averages over the visibilities, the total weights for the
selected polarization product is calculated as:

.. math:: W_{\mathrm{tot}} = \sum_{c,a} W[p',c,a,0]

Step 1: Normalization and Frequency-to-Lag Transform
-----------------------------------------------------

The first computational step in the ``fourfit4`` fringe fitting algorithm,
transforms the visibility data from the frequency domain into a *single-band delay* (SBD) domain (indexed by `lag`). This operator,
called ``NormFX``, is implemented in ``MHO_SingleSidebandNormFX.cc`` for single-sideband data
and ``MHO_MixedSidebandNormFX`` for mixed (USB/LSB/DSB) data. This transformation is applied
only along the intra-channel FREQ_AXIS (indexed by spectral-bin *s*) of the visibility tensor.

Ideally, for intra-channel visibility data without any non-linear phase corruption, this operator
has the effect of placing all of the fringe 'power' of each channel into a single `lag` bin.
This makes further manipulation of the data during the (MBD, DR) search much more efficient.
However, it must be noted that all channels must be phase and delay corrected such
that they are aligned and share the same `lag` (SBD) bin. Otherwise, channels for which
power is distributed to a different SBD bin will be lost, and not contribute coherently
to the total fringe amplitude.

Zero-Padding
~~~~~~~~~~~~

Let :math:`N_s` denote the number of frequency spectral-bins. Then before the frequency to lag
transformation, a padding factor of :math:`P = 4` is applied: the FREQ\_AXIS is
extended to :math:`P \cdot N_s` elements by appending zeros at the end.
Zero-padding provides additional interpolation points in the lag domain,
improving the resolution of the SBD peak search, and reducing the power which
may bleed into other bins.

Forward Discrete Fourier Transform
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A one-dimensional forward DFT is applied along the (zero-padded) frequency axis
for each channel slice (p, c, a) of the visibilities, :math:`\mathbf{V}`.
For a each channel *c* with bandwidth, :math:`B_c`, at AP *a*, we apply:

.. math::

   \mathcal{S}[p,c,a,\ell] \;=\;
   \sum_{s=0}^{N_s-1} V[p,c,a,s] \;
   \exp\!\left(-\frac{2\pi i \, \ell \, s}{P \cdot N_s}\right)
   \qquad \ell = 0,\dots,P\cdot N_s-1

The result :math:`\mathcal{S}` is the *SBD array* (short for "single-band
delay"). Each lag :math:`\ell` corresponds to a time delay which is common
to all channels. The full search range (window) of the SBD is determined by the
channel bandwidth and the number of spectral points, :math:`N_s`, chosen during
correlation, and is given by :math:`\pm \frac{N_s}{2 B_c}`. The delay resolution
of the SBD axis is determined by the channel bandwidth and the padding factor (:math:`P = 4`),
and is given by :math:`\delta_{\mathrm{SBD}} = \frac{1}{P B_c}`. Note that this stage of
the search algorithm requires all channels to have the same bandwidth, or to have been
padded out to the same bandwidth, so that the SBD axis can be aligned across all channels.

Cyclic Rotation
~~~~~~~~~~~~~~~

The FFT output places the zero-delay (DC) term at index :math:`\ell=0`. To center
the lag array around zero delay, a cyclic shift by :math:`P\cdot N_s / 2` positions
is applied along the FREQ\_AXIS:

.. math:: \mathcal{S}'[\ell] = \mathcal{S}\bigl[(\ell + P N_s/2) \bmod (P N_s)\bigr]

After rotation, negative delays appear in the first half of the array and
positive delays in the second half, with zero delay at the center. Note that this
operation is essentially the same as implemented by the matlab/python function ``fftshift``.

Lower-Sideband Conjugation
~~~~~~~~~~~~~~~~~~~~~~~~~~

Channels flagged as lower sideband (LSB, ``net_sideband = "L"``) have an
inverted frequency axis relative to upper sideband (USB) data. To correct for
this, the SBD slice for each LSB channel is complex-conjugated:

.. math::

   \mathcal{S}'[p,\,c_{\mathrm{LSB}},\,a,\,\ell] \;\leftarrow\;
   \overline{\mathcal{S}'[p,\,c_{\mathrm{LSB}},\,a,\,\ell]}

This is equivalent to flipping the frequency axis before the FFT, ensuring that
USB and LSB channels contribute coherently to the same delay space.

Normalization
~~~~~~~~~~~~~

Finally, the entire SBD array is divided by the original (unpadded) number of frequency
spectral-bins:

.. math:: \mathcal{S}'[p,c,a,\ell] \;\leftarrow\; \frac{\mathcal{S}'[p,c,a,\ell]}{N_s}

This normalization compensates for the FFT summation over :math:`N_s` terms. Neither the library FFTW3 nor the
native FFT implementation apply this normalization by-default, so it must be applied after the FFT.

Output Dimensions
~~~~~~~~~~~~~~~~~

The SBD array, :math:`\mathcal{S}'`, has dimensions ``[1, N_c, N_a, P*N_s]`` (one polarization
product, :math:`N_c` channels, :math:`N_a` APs, and :math:`P\cdot N_s` lags). The labels
associated with the 4-th axis are now physical delay values in microseconds.

Step 2: Coarse Three-Dimensional Grid Search
---------------------------------------------

Once the SBD array, :math:`\mathcal{S}'`, is generated, the coarse search,
implemented in ``MHO_MBDelaySearch.cc``, iterates over all SBD lags and,
for each lag, computes a two-dimensional function of (delay rate, multi-band delay)
using FFT-based methods. The result is a three-dimensional search space indexed
by (SBD, DR, MBD) from which the global maximum amplitude is identified.

Frequency Grid Construction for MBD Search
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Before the search, a *uniform frequency grid* is constructed from the
channel sky frequencies (file: ``MHO_UniformGridPointsCalculator.cc``).
This grid maps each channel to a discrete frequency bin in the array pre-transformation to MBD.
It constructs this grid from the following rules:

1. **Deduplication:** Adjacent frequencies within a tolerance
   :math:`\varepsilon = 10^{-4}` MHz are merged/summed. This handles DSB channel pairs
   that share the same sky frequency at one channel edge.

2. **Grid spacing:** Starting from the minimum channel spacing, the
   algorithm iteratively determines a grid spacing :math:`\delta_\nu` such that all
   channel frequencies fall on integer grid points (within tolerance).

3. **Grid size:** The grid starts at 2 points and doubles until it covers the
   full channel-frequency index range, then is extended to the next power of two
   and zero-padded by a factor of 4 for interpolation. The final grid has :math:`N_{\mathrm{grid}}`
   points (capped at 8192).

The *delay ambiguity* is a function of the grid spacing, and given by:

.. math:: \tau_{\mathrm{amb}} = \frac{1}{\delta_\nu}

which is the maximum unambiguous delay range (in :math:`\mu\mathrm{s}`). Note that each channel *c* maps
to a specific MBD grid bin via a precomputed lookup table stored in
``MHO_MBDelaySearch::fMBDBinForChannel[c]``.

Delay-Rate Transform
~~~~~~~~~~~~~~~~~~~~

For each SBD lag :math:`\ell`, the algorithm first extracts a slice of the SBD
array (with free running indices over channels, *c*, and APs, *a*):

.. math:: \mathcal{D}[c,\,a] = \mathcal{S}'[0,\,c,\,a,\,\ell]

This slice is then transformed to the fringe-rate domain via a FFT along the
TIME_AXIS (APs).

Let :math:`N_a` be the number of APs. Then the fringe-rate search space size is set to:

.. math:: N_{\mathrm{DRSP}} = 2 \cdot 2^{\lceil \log_2(N_a) \rceil}

and the actual FFT is performed on a further zero-padded array of size :math:`N_{\mathrm{fft}} = 4 \cdot N_{\mathrm{DRSP}}`.

This transformation is implemented in (file: ``MHO_DelayRate.cc``), and consists
of the following steps:

1. Zero-pad the time axis out to size :math:`N_{\mathrm{fft}}`.
2. Apply weights: :math:`\mathcal{D}[c,a] \leftarrow \mathcal{D}[c,a] \cdot W[0,c,a,0]`.
3. Apply a forward FFT along the time axis.
4. Apply a cyclic rotation by :math:`N_{\mathrm{fft}}/2`, to center the rate axis at 0.
5. Resample each channel's rate spectrum onto a common delay-rate grid of
   :math:`N_{\mathrm{DRSP}}` bins, by linear interpolation, at positions scaled
   by :math:`\nu_c/\nu_{\mathrm{ref}}` (see below).

The re-sampling done in step 5 (not just a decimation) is needed because the fringe rate
observed in channel *c* is the product of the physical delay rate and that channel's sky frequency.
On account of this, the same physical delay rate appears in a *different* fringe-rate bin in every channel.
In order to align the channels onto the same grid, each channel's spectrum needs to be
resampled with a scaling factor :math:`\nu_c/\nu_{\mathrm{ref}}`. The positions at which the
cyclically-rotated spectrum of step 4 is sampled are given by:

.. math::

   \lambda_c(k) = \left[ \left(k - \frac{N_{\mathrm{DRSP}}}{2}\right) b_c
   + \frac{N_{\mathrm{fft}}}{2} \right] \bmod N_{\mathrm{fft}},
   \qquad
   b_c = \frac{\nu_c}{\nu_{\mathrm{ref}}} \cdot
   \frac{N_{\mathrm{fft}}}{N_{\mathrm{DRSP}}}
   = 4\,\frac{\nu_c}{\nu_{\mathrm{ref}}}

and the output :math:`\mathcal{R}[c,k]` (below) is formed via linear
interpolation between the two samples bracketing :math:`\lambda_c(k)`. The effect is
that channels above the reference frequency are compressed and channels below
it are stretched, by a factor that maps their channel dependent fringe rates to a common
grid at the reference frequency. This must be done before the scatter-accumulate in the following section
so that the *k*-th bin maps to the same physical delay rate for every channel, and
the contributions from channels at different sky frequencies can be summed coherently.

After step 5, the axis labels corresponding to each bin *k* are fringe rates *at the reference
frequency* (units :math:`\mathrm{s}^{-1}`), given by:

.. math::

   \mathrm{FR}[k] = \frac{k - N_{\mathrm{DRSP}}/2}{\Delta t \cdot N_{\mathrm{DRSP}}}
   \qquad k = 0, \dots, N_{\mathrm{DRSP}}-1

where :math:`\Delta t` is the AP length (chosen at time of correlation) in seconds.
The final conversion to delay rate, :math:`\mathrm{DR}[k] = \frac{ \mathrm{FR}[k] }{ \nu_{\mathrm{ref}} }`,
is applied in ``MHO_MBDelaySearch.cc``, and the delay-rate bin spacing is given by:

.. math:: \delta_{\mathrm{DR}} = \frac{1}{\Delta t \, N_{\mathrm{DRSP}} \, \nu_{\mathrm{ref}}}

The maximum span of the available
delay-rate search range is: :math:`\pm \frac{1}{2\Delta t \nu_{\mathrm{ref}} }`
(which can be limited by control keyword ``dr_win`` to a narrower range).

Scatter-Accumulate into MBD Search Buffer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After the delay-rate transform, the data of this slice is a two-dimensional array
:math:`\mathcal{R}[c,\,k]` indexed by channel and delay-rate bin. Each channel's
contribution is accumulated into the grid bin corresponding to its sky frequency as:

.. math::

   \mathcal{B}[k,\,g] \;\mathrel{+}=\; \mathcal{R}[c,\,k]
   \quad \text{where} \quad
   g = \text{fMBDBinForChannel}[c]

The buffer :math:`\mathcal{B}` has dimensions :math:`[N_{\mathrm{DRSP}},\, N_{\mathrm{grid}}]`.
Channels at different sky frequencies contribute to different grid bins, while DSB channels
contribute equally to the same bin. In this way the multi-channel data fills in a (often sparse)
set of bins before the transformation to MBD space.

FFT to MBD Space
~~~~~~~~~~~~~~~~

A forward FFT is applied along axis 1, of the buffer :math:`\mathcal{B}`:

.. math::

   \mathcal{M}[k,\,m] =
   \sum_{g=0}^{N_{\mathrm{grid}}-1}
   \mathcal{B}[k,\,g] \;
   \exp\!\left(-\frac{2\pi i \, m \, g}{N_{\mathrm{grid}}}\right)

This transforms from the frequency/channel domain to the MBD delay domain.
The MBD axis values are obtained from the axis labels after a final cyclic
rotation by :math:`N_{\mathrm{grid}}/2`, to center the zero-delay bin. The MBD
bin spacing is set by the frequency grid, and is given by:

.. math::

   \delta_{\mathrm{MBD}} = \frac{1}{N_{\mathrm{grid}} \, \delta_\nu}
   = \frac{\tau_{\mathrm{amb}}}{N_{\mathrm{grid}}}

Optional Incoherent Averaging
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a coherence time :math:`t_{\mathrm{cohere}} > 0` is specified, a box-car
smoothing is applied along the delay-rate axis *before* the maximum
search. The half-width in bins is:

.. math::

   n_{\mathrm{box}} = \mathrm{round}\!\left(
       \frac{N_{\mathrm{DRSP}} \cdot \Delta t}{2 \cdot t_{\mathrm{cohere}}}
   \right)

The smoothing is applied to the amplitudes :math:`|\mathcal{M}[k,m]|`, and the
result is stored back as real values (Note: the subsequent maximum search uses
``std::norm``, which remains monotonic). This step is normally skipped by default,
and is only triggered by the presence of the control file keyword `t_cohere`.

Maximum Search
~~~~~~~~~~~~~~

After the transformation to MBD space, the program iterates over all (k, m) points
for each SBD lag, computing the squared magnitude :math:`|\mathcal{M}[k,m]|^2` and
tracking the global maximum. User-specified search windows (SBD, MBD, DR) can restrict
the domain over which this search is performed.

The output of the coarse search is:

- :math:`(\ell^*, k^*, m^*)` -- integer bin indices of the global maximum
  (SBD, DR, MBD)
- :math:`A_{\max}` -- the amplitude at the peak (normalized by :math:`W_{\mathrm{tot}}`)
- The search axes and bin spacings

Step 3: Fine Peak Interpolation
-------------------------------

The coarse search yields integer bin indices. The fine interpolation
(file: ``MHO_InterpolateFringePeakOptimized.cc``) refines the peak
location to sub-bin precision using a two-step process: (1) direct evaluation
of the fringe function on a 5x5x5 grid, followed by (2)
iterative quintic Lagrange interpolation.

The 5x5x5 Grid
~~~~~~~~~~~~~~

The grid is centered on the coarse peak :math:`(\ell^*, k^*, m^*)` with offsets:

- **SBD:** 5 points at :math:`\ell^* - 2,\;\dots,\; \ell^* + 2`
  (full-bin steps :math:`\delta_{\mathrm{SBD}}`)
- **MBD:** 5 points at :math:`m^* - 2,\;\dots,\; m^* + 2`
  (half-bin steps :math:`0.5 \cdot \delta_{\mathrm{MBD}}`)
- **DR:**  5 points at :math:`k^* - 2,\;\dots,\; k^* + 2`
  (half-bin steps :math:`0.5 \cdot \delta_{\mathrm{DR}}`)

This requires 125 trial points to fill the interpolation grid.

Evaluating Each Grid Point
~~~~~~~~~~~~~~~~~~~~~~~~~~

For each trial point :math:`(\ell_{\mathrm{trial}},\, m_{\mathrm{trial}},\, k_{\mathrm{trial}})`,
the algorithm computes the weighted, counter-rotated coherent sum over all
channels and APs:

.. math::

   Z = \frac{1}{W_{\mathrm{tot}}}
   \sum_{c=0}^{N_c-1} \sum_{a=0}^{N_a-1}
   W[p',c,a,0] \; \mathcal{S}'[p',c,a,\ell_{\mathrm{trial}}] \;
   \Psi(c, a, \tau_{\mathrm{mbd}}, \dot{\tau}_{\mathrm{dr}})

The *fringe rotation phasor* :math:`\Psi` removes the residual delay and delay-rate
terms from the visibilities:

.. math::

   \Psi(c, a, \tau_{\mathrm{mbd}}, \dot{\tau}_{\mathrm{dr}})
   = \exp\!\Bigl(-2\pi i \bigl[
       \nu_c \, \dot{\tau}_{\mathrm{dr}} \, \Delta t_a
       + \tau_{\mathrm{mbd}} \, (\nu_c - \nu_{\mathrm{ref}})
   \bigr]\Bigr)

where:

- :math:`\nu_c` is the sky frequency of channel *c* (MHz)
- :math:`\dot{\tau}_{\mathrm{dr}}` is the trial delay rate (:math:`\mu\mathrm{s}/\mathrm{s}`)
- :math:`\Delta t_a = t_a + \Delta t/2 - t_{\mathrm{FRT}}` is the time offset
  of AP *a* from the fourfit reference time (FRT), in seconds
- :math:`\tau_{\mathrm{mbd}}` is the trial multi-band delay (us)
- :math:`\nu_{\mathrm{ref}}` is the reference frequency (MHz)

Precomputation and Phasor Recurrence
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To avoid computing expensive :math:`\exp(\cdot)` in the inner loop, the algorithm
precomputes:

- **Channel phasors** :math:`\Phi_c^{\mathrm{MBD}}[m_{\mathrm{trial}}]` for
  each MBD trial point and channel (the frequency-offset part of the phasor)
- **Step phasors** :math:`\Phi_c^{\mathrm{step}}[k_{\mathrm{trial}}] =
  \exp(-2\pi i \, \nu_c \, \dot{\tau}_{\mathrm{dr}} \, \Delta t)` for each
  DR trial point and channel
- **Initial phasors** :math:`\Phi_c^{\mathrm{init}}[k_{\mathrm{trial}}] =
  \exp(-2\pi i \, \nu_c \, \dot{\tau}_{\mathrm{dr}} \, \Delta t_0)`

The phasor for AP *a* is then advanced by recurrence:

.. math:: \Psi_c(a+1) = \Psi_c(a) \cdot \Phi_c^{\mathrm{step}}

This replaces :math:`N_a` complex exponential evaluations with :math:`N_a` complex multiplications.

Five-point Lagrange Interpolation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After filling the 5x5x5 amplitude cube:

.. math:: \mathcal{F}[i,j,k] = |Z_{i,j,k}| \qquad i=0..4,\; j=0..4,\; k=0..4

the algorithm performs an iterative refinement search (function ``max555``), which:

1. Initializes the search location to the cube center, and step size ``dx = 0.4``, in each
   dimension.
2. Defines an 11x11x11 sub-grid within the current bounds.
3. For each of the 1331 sub-grid points, computes the interpolated value
   using *five point (quartic) Lagrange interpolation* [AS1965]_ (25.2.15).
4. Locates the maximum value and its coordinates.
5. Relocate the center to the maximum, then reduces ``dx`` by a factor of 5.
6. Repeats until ``dx < 1e-4`` in all dimensions.

The three-dimensional interpolated value of the fringe amplitude at :math:`(x_0, x_1, x_2)` is given by:

.. math::

   \mathcal{F}_{\mathrm{interp}}(x_0, x_1, x_2) =
   \sum_{i=0}^{4}\sum_{j=0}^{4}\sum_{k=0}^{4}
   a_i^{(0)} \, a_j^{(1)} \, a_k^{(2)} \, \mathcal{F}[i,j,k]

using the following definition for the one-dimensional Lagrange coefficients at fractional position *p*:

.. math::

   \begin{aligned}
   a_0 &= \frac{(p^2-1)\,p\,(p-2)}{24}, &
   a_1 &= -\frac{(p-1)\,p\,(p^2-4)}{6}, \\
   a_2 &= \frac{(p^2-1)\,(p^2-4)}{4}, &
   a_3 &= -\frac{(p+1)\,p\,(p^2-4)}{6}, \\
   a_4 &= \frac{(p^2-1)\,p\,(p+2)}{24}
   \end{aligned}

The final value for the peak location in interpolation space is given by:

.. math::

   (\xi_0, \xi_1, \xi_2)
   =
   \underset{(x_0,x_1,x_2)}{\arg\max}
   \;
   \mathcal{F}_{\mathrm{interp}}(x_0, x_1, x_2)

Refined Peak Parameters
~~~~~~~~~~~~~~~~~~~~~~~

The final refined (but dimensionless) coordinates :math:`(\xi_0, \xi_1, \xi_2)` are converted to
physical units as follows:

.. math::

   \tau_{\mathrm{SBD}} &= \mathrm{SBD}[\ell^*] + \xi_0 \cdot \delta_{\mathrm{SBD}} \\
   \tau_{\mathrm{MBD}} &= \mathrm{MBD}[m^*] + \xi_1 \cdot 0.5 \cdot \delta_{\mathrm{MBD}} \\
   \dot{\tau}_{\mathrm{DR}} &= \mathrm{DR}[k^*] + \xi_2 \cdot 0.5 \cdot \delta_{\mathrm{DR}} \\
   f_{\mathrm{rate}} &= \dot{\tau}_{\mathrm{DR}} \cdot \nu_{\mathrm{ref}} \\
   A_{\mathrm{fringe}} &= \mathcal{F}_{\max} \quad \text{(interpolated peak amplitude)}


Step 4: Compute Output
----------------------

Once the location of the fringe peak has been determined. A collection of summary quantities
and plots are calculated in order that the user may evaluate the quality of the fit.
See the section :ref:`output` for details.


Other Implementation Details
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Ionospheric Fringe Fitting
^^^^^^^^^^^^^^^^^^^^^^^^^^

For VGOS geodetic VLBI removing the effect of the ionosphere
is an essential step during fringe-fitting. This modified algorithm is
implemented in ``MHO_IonosphericFringeFitter``.
See the section :ref:`ionospheric` for details.


Parallel Backends
^^^^^^^^^^^^^^^^^

The coarse MBD search supports three parallel backends selected at compile time:

- **Sequential** (``MHO_MBDelaySearch``) -- single-threaded (default)
- **OpenMP** (``MHO_MBDelaySearchOpenMP``) -- shared-memory
  parallelism over the SBD lag loop
- **CUDA** (``MHO_MBDelaySearchCUDA``) -- GPU acceleration

FFT Backend
^^^^^^^^^^^

When FFTW3 is available, the algorithm uses the FFTW library for all DFT
transformations. Otherwise, an internal FFT implementation (which may be slower)
(``MHO_MultidimensionalFastFourierTransform``) is used.
If CUDA is enabled, CUFFT is used, but only during the coarse MBD search.

Mixed-Sideband Handling
^^^^^^^^^^^^^^^^^^^^^^^

When the data contains a mixture of USB and LSB channels (or DSB channel
pairs), the algorithm uses ``MHO_MixedSidebandNormFX`` instead of
``MHO_SingleSidebandNormFX``. The mixed-sideband version applies
frequency-axis flipping and conjugation on a per-channel basis *before*
the FFT, ensuring that all channels contribute coherently to the same delay
space.

Single-Channel Degeneracy
^^^^^^^^^^^^^^^^^^^^^^^^^

When only one active channel is present, the MBD is set equal to the SBD
(by definition, a single channel cannot distinguish between SBD and MBD).

Caching and Iteration
^^^^^^^^^^^^^^^^^^^^^

The fringe fitter supports iterative refinement loops controlled by
user-specified ``prefit`` and ``postfit`` operators. When
operators from both categories are present, the original visibility and weight data are cached
and can be refreshed between iterations, allowing convergence-based
outer loops (e.g., iterative flagging or passband estimation).


.. [#footnote1] Note that the distinction between the `reference` and `remote` station matters, since the reversal of the station assignment results in the complex conjugation of the visibility data.
