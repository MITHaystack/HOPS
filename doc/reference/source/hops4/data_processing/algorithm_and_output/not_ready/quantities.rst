==========================================================
MHO\_BasicFringeInfo: Quantities and Formulas
==========================================================

VLBI Fringe-Fitting Error Estimation
=====================================

This document describes the input and output quantities for every
function in the ``MHO_BasicFringeInfo`` class.  Symbols are
cross-referenced across functions: the same physical quantity always
carries the same symbol.

Master Symbol Table
===================

.. list-table:: Complete symbol table for all quantities.
   :widths: 18 40 12 30
   :header-rows: 1

   * - Symbol
     - Description
     - Units
     - Notes
   * - :math:`\nu`
     - observing frequency
     - Hz
     -
   * - :math:`\nu_\mathrm{ref}`
     - reference frequency
     - Hz
     - center of band
   * - :math:`\nu_0`
     - lowest channel edge frequency
     - Hz
     -
   * - :math:`\nu_k`
     - frequency of channel :math:`k`
     - Hz
     - :math:`k=0,\dots,N_\mathrm{chan}\!-\!1`
   * - :math:`\Delta\nu`
     - frequency spacing between channels
     - Hz
     -
   * - :math:`\Delta\nu_\mathrm{spread}`
     - total frequency spread
     - Hz
     - :math:`\nu_{\max}-\nu_{\min}`
   * - :math:`N_\mathrm{chan}`
     - number of frequency channels
     - --
     -
   * - :math:`N_\mathrm{pol}`
     - effective number of polarizations
     - --
     - 1 or 2
   * - :math:`N_\mathrm{ap}`
     - total number of accumulation periods
     - --
     -
   * - :math:`N_\mathrm{seg}`
     - number of time segments
     - --
     - switched-mode
   * - :math:`\Delta t_\mathrm{ap}`
     - accumulation period duration
     - s
     -
   * - :math:`\Delta t_\mathrm{samp}`
     - sampling period
     - s
     -
   * - :math:`T_\mathrm{int}`
     - total integration time
     - s
     - :math:`N_\mathrm{ap}\,\Delta t_\mathrm{ap}`
   * - :math:`A`
     - fringe amplitude (normalized)
     - --
     - :math:`0\le A\le 1`
   * - :math:`A_k`
     - channel :math:`k` phasor magnitude
     - --
     - :math:`|\,\text{chan\_phasors}[k]\,|`
   * - :math:`\mathcal{A}`
     - fringe amplitude in Whitney units
     - :math:`10^{-4}`
     - :math:`A\times 10^4`
   * - :math:`\mathrm{SNR}`
     - signal-to-noise ratio
     - --
     - dimensionless
   * - :math:`\mathrm{PFD}`
     - probability of false detection
     - --
     - :math:`0\le\mathrm{PFD}\le 1`
   * - :math:`N_\mathrm{pts}`
     - number of grid points searched
     - --
     -
   * - :math:`\tau_\mathrm{mbd}`
     - multi-band delay (residual)
     - s
     -
   * - :math:`\sigma_\mathrm{mbd}`
     - MBD error (no ionosphere)
     - s
     -
   * - :math:`\tau_\mathrm{sbd}`
     - sub-band delay (slope)
     - s
     -
   * - :math:`\sigma_\mathrm{sbd}`
     - SBD error
     - s
     -
   * - :math:`\Delta\tau_\mathrm{sbd}`
     - SBD separation (channel spacing in delay)
     - s
     -
   * - :math:`\dot{\tau}`
     - delay rate (d/dt of residual delay)
     - s/s
     -
   * - :math:`\sigma_{\dot{\tau}}`
     - delay-rate error
     - s/s
     -
   * - :math:`\phi`
     - residual phase
     - deg or rad
     -
   * - :math:`\sigma_\phi`
     - phase error
     - deg
     -
   * - :math:`\sigma_{\phi,\mathrm{delay}}`
     - phase-delay error
     - s
     -
   * - :math:`\overline{\mathrm{sb}}`
     - mean sideband weight
     - --
     - :math:`\in[-1,1]`
   * - :math:`\Delta\mathrm{TEC}`
     - differential TEC
     - TECU
     - :math:`10^{16}\,\mathrm{el/m^2}`
   * - :math:`\sigma_\tau`
     - ionospheric delay :math:`\sigma`
     - s
     -
   * - :math:`\sigma_{\phi}^{\mathrm{(ion)}}`
     - ionospheric phase :math:`\sigma`
     - rad
     -
   * - :math:`\sigma_{\mathrm{dTEC}}`
     - dTEC :math:`\sigma`
     - TECU
     -
   * - :math:`f_\mathrm{amp}`
     - total fringe amplitude
     - --
     - summed over channels
   * - :math:`\mathcal{C}`
     - ionospheric correlation matrix
     - --
     - :math:`3\times 3`
   * - ``text{sbavg}``
     - average sideband weight
     - --
     - same as :math:`\overline{\mathrm{sb}}`
   * - :math:`f_\mathrm{bw}`
     - bandwidth correction factor
     - --
     -
   * - :math:`f_\mathrm{ap}`
     - total AP fraction retained
     - --
     - flagged / unflagged
   * - :math:`W`
     - Whitney scale factor
     - --
     - :math:`10^4`
   * - :math:`b`
     - TEC unit-conversion constant
     - --
     - :math:`-1.3445`

Utility Functions
=================

leftpadzeros\_integer
---------------------

Formats an integer ``value`` as a zero-padded string of width
``n_places``.  Purely cosmetic; no VLBI-specific quantities.

make\_legacy\_datetime\_format
------------------------------

Converts a ``legacy_hops_date`` struct ``(h,m,s,d,y)`` into the HOPS3
string ``HHMMSS.xx``.  No scientific quantities.

make\_legacy\_datetime\_format\_v2
----------------------------------

Converts a ``legacy_hops_date`` into ``YYYY:DDD:HHMMSS``.  No
scientific quantities.

Signal-to-Noise Ratio
=====================

calculate\_snr
--------------

.. math::
   \sigma_{\mathrm{vis}}^{-1} &= f_1 \cdot f_2 \cdot f_3 \cdot
      \sqrt{\frac{\Delta t_{\mathrm{ap}}}{\Delta t_{\mathrm{samp}}}} \\
   \mathrm{SNR} &= \frac{f_{\mathrm{bw}} \cdot \mathcal{A} \cdot
      \sigma_{\mathrm{vis}}^{-1}}
      {W \cdot f_{\mathrm{amp\_corr}}}
      \sqrt{f_{\mathrm{ap}} \cdot N_{\mathrm{pol}}}

where :math:`f_1=1.0` (more than 16 lags), :math:`f_2=0.881` (2-bit
quantization loss), :math:`f_3=0.970` (DiFX correction),
:math:`W=10^4` (Whitney scale), and :math:`f_{\mathrm{amp\_corr}}=1.0`
(amplitude correction factor).

**Inputs:** :math:`N_{\mathrm{pol}}` (effective pols),
:math:`\Delta t_{\mathrm{ap}}` (AP period), :math:`\Delta
t_{\mathrm{samp}}` (sample period), :math:`f_{\mathrm{ap}}` (fraction
APs retained), :math:`\mathcal{A}` (amplitude in Whitney units),
:math:`f_{\mathrm{bw}}` (bandwidth correction).

**Output:** :math:`\mathrm{SNR}` (dimensionless).

Parameter Errors
================

calculate\_mbd\_no\_ion\_error
------------------------------

Error on the multi-band delay when no ionospheric term is fitted.

.. math::
   \sigma_{\mathrm{mbd}} = \frac{1}{2\pi \, \Delta\nu_{\mathrm{spread}} \,
      \mathrm{SNR}}

**Inputs:** :math:`\Delta\nu_{\mathrm{spread}}` (frequency spread, Hz),
:math:`\mathrm{SNR}` (from ``calculate_snr``).

**Output:** :math:`\sigma_{\mathrm{mbd}}` (s).

calculate\_sbd\_error
---------------------

Error on the sub-band delay.

.. math::
   \sigma_{\mathrm{sbd}} =
      \frac{\sqrt{12} \cdot \Delta\tau_{\mathrm{sbd}} \cdot 4}
      {2\pi \cdot \mathrm{SNR} \cdot \bigl(2 - |\text{sbavg}|\bigr)}

**Inputs:** :math:`\Delta\tau_{\mathrm{sbd}}` (SBD separation, s),
:math:`\mathrm{SNR}` (from ``calculate_snr``), ``sbavg`` (mean
sideband weight, same as :math:`\overline{\mathrm{sb}}`).

**Output:** :math:`\sigma_{\mathrm{sbd}}` (s).

calculate\_drate\_error\_v1
---------------------------

Delay-rate error expressed via accumulation-period bookkeeping.

.. math::
   \sigma_{\dot{\tau}} =
      \frac{\sqrt{12}}{2\pi \cdot \mathrm{SNR} \cdot \nu_{\mathrm{ref}} \cdot
         T_{\mathrm{int}}}
   \qquad
   T_{\mathrm{int}} = N_{\mathrm{ap}} \cdot \Delta t_{\mathrm{ap}}

**Inputs:** :math:`\mathrm{SNR}`, :math:`\nu_{\mathrm{ref}}`,
:math:`N_{\mathrm{ap}}`, :math:`\Delta t_{\mathrm{ap}}`.

**Output:** :math:`\sigma_{\dot{\tau}}` (s/s).

calculate\_drate\_error\_v2
---------------------------

Equivalent to v1 but takes :math:`T_{\mathrm{int}}` directly.

.. math::
   \sigma_{\dot{\tau}} =
      \frac{\sqrt{12}}{2\pi \cdot \mathrm{SNR} \cdot \nu_{\mathrm{ref}} \cdot
         T_{\mathrm{int}}}

**Inputs:** :math:`\mathrm{SNR}`, :math:`\nu_{\mathrm{ref}}`,
:math:`T_{\mathrm{int}}`.

**Output:** :math:`\sigma_{\dot{\tau}}` (s/s).

calculate\_pfd
--------------

Probability of false detection for a grid-search fringe detection.

.. math::
   \mathrm{PFD} = 1 - \Bigl[1 - \exp\!\Bigl(-\frac{\mathrm{SNR}^2}{2}\Bigr)
      \Bigr]^{N_{\mathrm{pts}}}

For :math:`\mathrm{PFD}<0.01` the small-argument approximation is used:
:math:`\mathrm{PFD} \approx N_{\mathrm{pts}} \exp(-\mathrm{SNR}^2/2)`.

**Inputs:** :math:`\mathrm{SNR}`, :math:`N_{\mathrm{pts}}` (grid
points searched).

**Output:** :math:`\mathrm{PFD}` (probability).

calculate\_phase\_error
-----------------------

Phase error in degrees (no ionospheric term).

.. math::
   \sigma_{\phi} =
      \frac{180^\circ \cdot \sqrt{1 + 3\,\text{sbavg}^2}}{\pi \cdot
         \mathrm{SNR}}

The factor :math:`\sqrt{1+3\,\text{sbavg}^2}` accounts for dual-sideband
weighting (unity for single-sideband, :math:`2` for full DSB).

**Inputs:** ``sbavg``, :math:`\mathrm{SNR}`.

**Output:** :math:`\sigma_{\phi}` (degrees).

calculate\_phase\_delay\_error
------------------------------

Phase-delay error in seconds (no ionospheric term).

.. math::
   \sigma_{\phi,\mathrm{delay}} =
      \frac{\sqrt{1 + 3\,\text{sbavg}^2}}{2\pi \cdot \mathrm{SNR} \cdot
         \nu_{\mathrm{ref}}}

**Inputs:** ``sbavg``, :math:`\mathrm{SNR}`, :math:`\nu_{\mathrm{ref}}`.

**Output:** :math:`\sigma_{\phi,\mathrm{delay}}` (s).

Theoretical Residual Scatter
============================

calculate\_theory\_timerms\_phase
---------------------------------

Expected RMS scatter of phase residuals in the *time* domain.

.. math::
   \mathrm{timerms}_{\phi} =
      \frac{\sqrt{N_{\mathrm{seg}}} \cdot 180^\circ}{\pi \cdot
         \mathrm{SNR}}

**Inputs:** :math:`N_{\mathrm{seg}}` (effective segments),
:math:`\mathrm{SNR}`.

**Output:** :math:`\mathrm{timerms}_{\phi}` (degrees).

calculate\_theory\_timerms\_amp
-------------------------------

Expected RMS scatter of amplitude residuals in the *time* domain.

.. math::
   \mathrm{timerms}_{A} = \mathrm{timerms}_{\phi} \cdot \frac{\pi}{180^\circ}
      \cdot 100

**Inputs:** :math:`N_{\mathrm{seg}}`, :math:`\mathrm{SNR}`.

**Output:** :math:`\mathrm{timerms}_{A}` (percent).

calculate\_theory\_freqrms\_phase
---------------------------------

Expected RMS scatter of phase residuals in the *frequency* domain.

.. math::
   \mathrm{freqrms}_{\phi} =
      \frac{\sqrt{N_{\mathrm{chan}}} \cdot 180^\circ}{\pi \cdot
         \mathrm{SNR}}

**Inputs:** :math:`N_{\mathrm{chan}}`, :math:`\mathrm{SNR}`.

**Output:** :math:`\mathrm{freqrms}_{\phi}` (degrees).

calculate\_theory\_freqrms\_amp
-------------------------------

Expected RMS scatter of amplitude residuals in the *frequency* domain.

.. math::
   \mathrm{freqrms}_{A} = \mathrm{freqrms}_{\phi} \cdot \frac{\pi}{180^\circ}
      \cdot 100

**Inputs:** :math:`N_{\mathrm{chan}}`, :math:`\mathrm{SNR}`.

**Output:** :math:`\mathrm{freqrms}_{A}` (percent).

Phase Correction and Ionospheric Covariance
============================================

correct\_phases\_mbd\_anchor\_sbd
----------------------------------

When the MBD anchor is set to ``sbd`` (rather than ``model``), a phase
correction is applied to both the total and residual phases:

.. math::
   \delta f &= \bigl(\nu_{\mathrm{ref}} - \nu_0\bigr) \bmod \Delta\nu \\
   \phi_{\mathrm{tot}} &\leftarrow
      \bigl(\phi_{\mathrm{tot}} + 360^\circ \cdot \tau_{\mathrm{mbd}} \cdot
      \delta f\bigr) \bmod 360^\circ \\
   \phi_{\mathrm{res}} &\leftarrow
      \bigl(\phi_{\mathrm{res}} + 360^\circ \cdot \tau_{\mathrm{mbd}} \cdot
      \delta f\bigr) \bmod 360^\circ

**Inputs:** :math:`\nu_{\mathrm{ref}}`, :math:`\nu_0`, :math:`\Delta\nu`,
:math:`\tau_{\mathrm{mbd}}` (named ``delta_mbd`` in code),
:math:`\phi_{\mathrm{tot}}` (``totphase_deg``),
:math:`\phi_{\mathrm{res}}` (``resphase_deg``).

**Outputs:** :math:`\phi_{\mathrm{tot}}`, :math:`\phi_{\mathrm{res}}`
(modified in-place, degrees).

ion\_covariance
---------------

Computes the :math:`3\times 3` ionospheric covariance matrix for the
three fitted parameters: delay :math:`\tau`, residual phase :math:`\phi`,
and differential TEC (:math:`\Delta\mathrm{TEC}`).

For each channel :math:`k` the per-channel weight is

.. math::
   \sigma_k = \frac{\sqrt{N_{\mathrm{chan}}}\; f_{\mathrm{amp}}}
      {2\pi \cdot \mathrm{SNR} \cdot A_k},
   \qquad
   w_k = \frac{1}{\sigma_k^2}

where :math:`A_k = |\,\text{chan\_phasors}[k]\,|` is the channel phasor
magnitude.  The normal-equation matrix accumulates as

.. math::
   A_{00} &+= w_k\,(\nu_k' - \nu_0')^2 \\
   A_{01} &+= w_k\,(\nu_k' - \nu_0') \\
   A_{02} &+= w_k\,b\,\frac{\nu_k' - \nu_0'}{\nu_k'} \\
   A_{11} &+= w_k \\
   A_{12} &+= w_k\,\frac{b}{\nu_k'} \\
   A_{22} &+= w_k\,\Bigl(\frac{b}{\nu_k'}\Bigr)^2

with :math:`\nu_k' = 10^{-3}\nu_k` (GHz),
:math:`\nu_0' = 10^{-3}\nu_{\mathrm{ref}}` (GHz), and :math:`b=-1.3445`.
The covariance matrix :math:`\mathbf{C}=\mathbf{A}^{-1}` gives the
standard deviations :math:`\sigma_i = \sqrt{C_{ii}}` for
:math:`(\tau_{\mathrm{ion}},\;\phi_{\mathrm{ion}},\;\Delta\mathrm{TEC})`.
The matrix is then normalized to produce the correlation matrix
:math:`\mathcal{C}_{ij} = C_{ij}/(\sigma_i\sigma_j)`.

**Inputs:** :math:`N_{\mathrm{chan}}` (``nfreq``),
:math:`f_{\mathrm{amp}}` (``famp``), :math:`\mathrm{SNR}`,
:math:`\nu_{\mathrm{ref}}` (``ref_freq``), :math:`\{\nu_k\}`
(``chan_freqs``), :math:`\{\text{chan\_phasors}[k]\}` (complex).

**Output:**
:math:`\{\sigma_\tau,\;\sigma_{\phi}^{\mathrm{(ion)}},\;
\sigma_{\mathrm{dTEC}}\}` (``ion_sigmas``, length 3).

Cross-Reference Summary
=======================

.. list-table:: Data-flow cross-reference between functions.
   :widths: 28 22 50
   :header-rows: 1

   * - Producer
     - Quantity
     - Consumer(s)
   * - ``calculate_snr``
     - :math:`\mathrm{SNR}`
     - *all error functions below*
   * - ``calculate_mbd_no_ion_error``
     - :math:`\sigma_{\mathrm{mbd}}`
     - --
   * - ``calculate_sbd_error``
     - :math:`\sigma_{\mathrm{sbd}}`
     - --
   * - ``calculate_drate_error``
     - :math:`\sigma_{\dot{\tau}}`
     - --
   * - ``calculate_pfd``
     - :math:`\mathrm{PFD}`
     - --
   * - ``calculate_phase_error``
     - :math:`\sigma_{\phi}`
     - --
   * - ``calculate_phase_delay_error``
     - :math:`\sigma_{\phi,\mathrm{delay}}`
     - --
   * - ``calculate_theory_timerms_phase``
     - :math:`\mathrm{timerms}_{\phi}`
     - ``calculate_theory_timerms_amp``
   * - ``calculate_theory_freqrms_phase``
     - :math:`\mathrm{freqrms}_{\phi}`
     - ``calculate_theory_freqrms_amp``
   * - ``correct_phases_mbd_anchor_sbd``
     - :math:`\phi_{\mathrm{tot}}`, :math:`\phi_{\mathrm{res}}`
     - --
   * - ``ion_covariance``
     - :math:`\sigma_\tau,\sigma_{\phi}^{\mathrm{(ion)}},\sigma_{\mathrm{dTEC}}`
     - --
