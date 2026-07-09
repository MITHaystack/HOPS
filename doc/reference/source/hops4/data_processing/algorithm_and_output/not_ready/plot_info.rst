==========================================================
MHO\_ComputePlotData: Quantities and Formulas
==========================================================

Fourfit Plot Data Computation
==============================

This document describes the input and output quantities for every
function in the ``MHO_ComputePlotData`` class.  Symbols are
cross-referenced with ``quants.rst`` (MHO\_BasicFringeInfo) so that
shared quantities like SNR, MBD, SBD, delay rate, and theoretical RMS
use the same notation.

Each function is linked to the Python plotting routine that consumes its
output (``fourfit_plot.py``).

Master Symbol Table
===================

.. list-table:: Symbol table for MHO_ComputePlotData.
   :widths: 18 40 12 30
   :header-rows: 1

   * - Symbol
     - Description
     - Units
     - Notes
   * - :math:`V_{p,c,t,b}`
     - raw visibility at (p,c,t,b)
     - --
     - complex
   * - :math:`W_{p,c,t}`
     - weight at (p,c,t)
     - --
     - scalar
   * - :math:`\tilde{V}_{p,c,t,b}`
     - corrected (de-rotated) visibility
     - --
     - complex
   * - :math:`V_{\mathrm{sbd},p,c,t,b}`
     - SBD-array (lag-domain) visibility
     - --
     - complex
   * - :math:`b`
     - SBD-bin index
     - --
     - :math:`0\dots N_{\mathrm{bin}}\!-\!1`
   * - :math:`b_{\max}`
     - SBD bin of maximum amplitude
     - --
     - fitted solution
   * - :math:`p`
     - polarization-product index
     - --
     - 0 (single pol product)
   * - :math:`c`
     - channel index
     - --
     - :math:`0\dots N_{\mathrm{chan}}\!-\!1`
   * - :math:`t`
     - accumulation-period index
     - --
     - :math:`0\dots N_{\mathrm{ap}}\!-\!1`
   * - :math:`N_{\mathrm{bin}}`
     - number of SBD bins
     - --
     - :math:`=2\cdot n_{\mathrm{lag}}`
   * - :math:`n_{\mathrm{lag}}`
     - number of lags (half the SBD bins)
     - --
     -
   * - :math:`\Gamma_{c,t}`
     - visibility rotation factor
     - --
     - :math:`\exp(+2\pi i \Phi_{c,t})`
   * - :math:`\Gamma^{\mathrm{MBD0}}_{c,t}`
     - vrot with MBD=0 (default SBD params)
     - --
     -
   * - :math:`\Gamma^{\mathrm{phase}}_{c,t}`
     - vrot with proper SBD params and fitted MBD
     - --
     -
   * - :math:`\Phi_{c,t}`
     - rotation phase for channel :math:`c`, AP :math:`t`
     - rad
     -
   * - :math:`\nu_c`
     - frequency of channel :math:`c`
     - Hz
     - from ``quants.rst``
   * - :math:`\Delta\nu_{\mathrm{sbd}}`
     - SBD-bin spacing
     - Hz
     -
   * - :math:`\mathrm{sb}_c`
     - net sideband of channel :math:`c`
     - --
     - :math:`\pm 1` or :math:`0` (DSB)
   * - :math:`\mathrm{BW}_c`
     - bandwidth of channel :math:`c`
     - Hz
     -
   * - :math:`A_{\mathrm{MBD}}(k)`
     - MBD amplitude at grid point :math:`k`
     - --
     - normalized
   * - :math:`\tau_k`
     - MBD grid value at point :math:`k`
     - s
     -
   * - :math:`A_{\mathrm{SBD}}(b)`
     - SBD amplitude at bin :math:`b`
     - --
     - normalized
   * - :math:`\tau_{\mathrm{sbd},b}`
     - SBD grid value at bin :math:`b`
     - :math:`\mu s`
     -
   * - :math:`A_{\mathrm{DR}}(k)`
     - delay-rate amplitude at FFT bin :math:`k`
     - --
     - normalized
   * - :math:`\dot{\tau}_k`
     - delay-rate grid value at bin :math:`k`
     - ns/s
     -
   * - :math:`N_{\mathrm{drsp}}`
     - DR-spectrum FFT size
     - --
     - power-of-2 :math:`\ge 256`
   * - :math:`\Delta t_{\mathrm{ap}}`
     - AP interval
     - s
     - from ``quants.rst``
   * - :math:`\Phi_{\mathrm{coh}}`
     - coherent average phase
     - rad
     - across all :math:`c,t`
   * - :math:`z_{c,t}`
     - per-channel segment phasor
     - --
     - :math:`V\!\cdot\!\Gamma^{\mathrm{phase}}`
   * - :math:`z_c^{\mathrm{fringe}}`
     - fringe phasor (time-avg per channel)
     - --
     - normalized
   * - :math:`N_{\mathrm{seg}}`
     - number of time segments
     - --
     -
   * - :math:`n_{\mathrm{ap/seg}}`
     - APs averaged per segment
     - --
     - integer
   * - :math:`A^{\mathrm{seg}}_{c,s}`
     - segment amplitude (channel :math:`c`, seg :math:`s`)
     - --
     -
   * - :math:`\phi^{\mathrm{seg}}_{c,s}`
     - segment phase (channel :math:`c`, seg :math:`s`)
     - rad
     -
   * - :math:`S^{\mathrm{xp}}_i`
     - cross-power spectrum element :math:`i`
     - --
     - complex
   * - :math:`|S^{\mathrm{xp}}_i|`
     - XP spectrum amplitude
     - --
     -
   * - :math:`\arg(S^{\mathrm{xp}}_i)`
     - XP spectrum phase
     - deg
     -
   * - :math:`x^{\mathrm{xp}}_i`
     - XP spectrum x-axis (MHz offset)
     - MHz
     -
   * - :math:`f_{\mathrm{coh}}`
     - coherent-averaging timescale
     - s
     - from fit control
   * - :math:`w^{\mathrm{USB}}_{c,s}`
     - USB weight fraction in segment (:math:`c,s`)
     - --
     - :math:`[0,1]`
   * - :math:`w^{\mathrm{LSB}}_{c,s}`
     - LSB weight fraction in segment (:math:`c,s`)
     - --
     - :math:`[0,1]`
   * - :math:`W_{\mathrm{tot}}`
     - total summed weight
     - --
     - :math:`\sum W`
   * - :math:`\mathrm{sbdbox}_c`
     - parabolic peak-interp. lag for ch :math:`c`
     - --
     - integer
   * - :math:`N_{\mathrm{USB}}`
     - count of USB channels
     - --
     -
   * - :math:`N_{\mathrm{LSB}}`
     - count of LSB channels
     - --
     -
   * - :math:`A^{\mathrm{chan}}_c`
     - time-averaged amplitude (all APs)
     - --
     - per channel
   * - :math:`\phi^{\mathrm{chan}}_c`
     - time-averaged phase (all APs)
     - rad
     - per channel
   * - :math:`\overline{A}_{\mathrm{inc}}^{\mathrm{freq}}`
     - incoherent avg amplitude (freq)
     - --
     - bias-corrected
   * - :math:`\overline{A}_{\mathrm{inc}}^{\mathrm{time}}`
     - incoherent avg amplitude (time)
     - --
     - bias-corrected
   * - ``QC``
     - quality code
     - --
     - character ``'0'``-``'9'``
   * - ``EC``
     - error code
     - --
     - ``G``, ``H``, or `` ``

Visibility Rotation Precomputation
===================================

The rotation factor :math:`\Gamma_{c,t}` removes the residual delay,
delay rate, and SBD so that the corrected visibility is stationary
(coherent).  Three precomputed tables exist:

.. math::
   \Gamma_{c,t} &= \exp\bigl(+2\pi i \cdot \text{vrot}(t,\nu_c,\nu_{\mathrm{ref}},
      \dot{\tau},\tau_{\mathrm{mbd}})\bigr) \\
   \Gamma^{\mathrm{MBD0}}_{c,t} &= \Gamma_{c,t}\big|_{\tau_{\mathrm{mbd}}=0,\;\text{default SBD params}} \\
   \Gamma^{\mathrm{phase}}_{c,t} &= \Gamma_{c,t}\big|_{\text{proper SBD params, fitted }\tau_{\mathrm{mbd}}}

precompute\_chan\_metadata
--------------------------

Reads channel frequency :math:`\nu_c`, net sideband
:math:`\mathrm{sb}_c\in\{+1,0,\!-\!1\}`, and bandwidth :math:`\mathrm{BW}_c`
for every channel.

**Inputs:** SBD-array channel axis labels (``net_sideband``,
``dsb_partner``, ``bandwidth``).

**Outputs:** :math:`\{\nu_c\}`, :math:`\{\mathrm{sb}_c\}`,
:math:`\{\mathrm{BW}_c\}` for :math:`c=0\dots N_{\mathrm{chan}}\!-\!1`.

precompute\_vr\_tables
----------------------

Populates :math:`\Gamma_{c,t}`, :math:`\Gamma^{\mathrm{MBD0}}_{c,t}`,
and :math:`\Gamma^{\mathrm{phase}}_{c,t}` for all :math:`(c,t)`.  The
default-SBD parameters are used for the first two (matching the state of
``fRot`` before ``calc_phase`` reconfigures the SBD setters); the proper
SBD parameters for the third.

**Inputs:** :math:`\{\nu_c\}`, :math:`\{\mathrm{sb}_c\}`,
:math:`\Delta t_{\mathrm{ap}}`, :math:`\Delta\nu_{\mathrm{sbd}}`,
:math:`\nu_{\mathrm{ref}}`, :math:`\dot{\tau}`, :math:`\tau_{\mathrm{mbd}}`,
:math:`\tau_{\mathrm{sbd}}`, :math:`b_{\max}`, ``frt_offset``.

**Outputs:** :math:`\{\Gamma_{c,t}\}`, :math:`\{\Gamma^{\mathrm{MBD0}}_{c,t}\}`,
:math:`\{\Gamma^{\mathrm{phase}}_{c,t}\}` arrays of size
:math:`N_{\mathrm{chan}}\!\times\! N_{\mathrm{ap}}`.

Spectrum Computation Functions
===============================

calc\_mbd
---------

Computes the multi-band delay amplitude spectrum.  For each channel the
data at the SBD-maximum bin is weighted, multiplied by
:math:`\Gamma^{\mathrm{MBD0}}_{c,t}`, and summed over all APs.  The
per-channel sums are placed into a uniform delay grid, FFT'd, and
cyclically rotated to produce :math:`A_{\mathrm{MBD}}(k)`.

**Inputs:** :math:`\{V_{\mathrm{sbd},p,c,t,b_{\max}}\}`,
:math:`\{W_{p,c,t}\}`, :math:`\{\Gamma^{\mathrm{MBD0}}_{c,t}\}`,
:math:`\{\nu_c\}`, :math:`W_{\mathrm{tot}}`.

**Outputs:** :math:`A_{\mathrm{MBD}}(k)` (amplitude), :math:`\tau_k`
(x-axis in :math:`\mu s`).

*Plot consumer:* ``make_dr_mbd_plot`` --- uses ``MBD_AMP`` and
``MBD_AMP_XAXIS`` (blue curve, top x-axis, units :math:`\mu s`).

calc\_sbd
---------

Computes the single-band delay amplitude spectrum.  For each SBD bin
:math:`b` all channels are summed (weighted, multiplied by
:math:`\Gamma_{c,t}`), and the amplitude is normalized by
:math:`W_{\mathrm{tot}}`.

**Inputs:** :math:`\{V_{\mathrm{sbd},p,c,t,b}\}`, :math:`\{W_{p,c,t}\}`,
:math:`\{\Gamma_{c,t}\}`, :math:`W_{\mathrm{tot}}`.

**Outputs:** :math:`A_{\mathrm{SBD}}(b)` (amplitude),
:math:`\tau_{\mathrm{sbd},b}` (x-axis in :math:`\mu s`).

*Plot consumer:* ``make_sbd_dtec_plot`` --- uses ``SBD_AMP`` and
``SBD_AMP_XAXIS`` (green curve, bottom x-axis, units :math:`\mu s`).

calc\_dr
--------

Computes the delay-rate amplitude spectrum.  Data at the SBD-maximum bin
is weighted, multiplied by :math:`\Gamma_{c,t}`, and summed over all
channels into a time-domain workspace of length :math:`N_{\mathrm{drsp}}`.
An FFT followed by a cyclic rotation yields :math:`A_{\mathrm{DR}}(k)`;
the x-axis is converted from time to delay-rate units via
:math:`\dot{\tau}_k = \tau_k / (\nu_{\mathrm{ref}}/1000)` (ns/s).  A
box-car smoothing kernel of width
:math:`n = N_{\mathrm{drsp}}\,\Delta t_{\mathrm{ap}} / f_{\mathrm{coh}}`
is applied when the ``t_cohere`` control parameter is set.

**Inputs:** :math:`\{V_{\mathrm{sbd},p,c,t,b_{\max}}\}`,
:math:`\{W_{p,c,t}\}`, :math:`\{\Gamma_{c,t}\}`, :math:`N_{\mathrm{ap}}`,
:math:`\Delta t_{\mathrm{ap}}`, :math:`\nu_{\mathrm{ref}}`,
:math:`W_{\mathrm{tot}}`, :math:`f_{\mathrm{coh}}`.

**Outputs:** :math:`A_{\mathrm{DR}}(k)` (amplitude), :math:`\dot{\tau}_k`
(x-axis in ns/s).

*Plot consumer:* ``make_dr_mbd_plot`` --- uses ``DLYRATE`` and
``DLYRATE_XAXIS`` (red curve, bottom x-axis, units ns/s).

calc\_phase
-----------

Computes the per-channel fringe phasor

.. math::
   z_c^{\mathrm{fringe}} = \frac{1}{W_c}\sum_t W_{p,c,t}\,
   V_{\mathrm{sbd},p,c,t,b_{\max}}\,\Gamma^{\mathrm{phase}}_{c,t}

and the coherent average phase

.. math::
   \Phi_{\mathrm{coh}} = \arg\!\Bigl(\sum_{c,t} W_{p,c,t}\,
   V_{\mathrm{sbd},p,c,t,b_{\max}}\,\Gamma^{\mathrm{phase}}_{c,t}\Bigr).

**Inputs:** :math:`\{V_{\mathrm{sbd},p,c,t,b_{\max}}\}`,
:math:`\{W_{p,c,t}\}`, :math:`\{\Gamma^{\mathrm{phase}}_{c,t}\}`,
:math:`\Delta\nu_{\mathrm{sbd}}`, :math:`\tau_{\mathrm{sbd}}`,
:math:`b_{\max}`.

**Outputs:** :math:`\{z_c^{\mathrm{fringe}}\}` (complex,
:math:`N_{\mathrm{chan}}`), :math:`\Phi_{\mathrm{coh}}` (rad).

*Plot consumer:* :math:`\Phi_{\mathrm{coh}}` is stored as
``/fringe/raw_resid_phase`` and used internally by ``calc_freqrms`` and
``calc_timerms``.  Not plotted directly.

calc\_xpower\_spec
------------------

Computes the cross-power spectrum.  For each lag :math:`i` the weighted,
de-rotated visibilities from all channels are summed, rearranged into the
array :math:`Y`, and FFT'd.  The result is scaled by a sideband-dependent
factor :math:`\sqrt{0.5}/(\pi W^{\mathrm{USB/LSB}}_{\mathrm{tot}})` and
phase-corrected by

.. math::
   \exp\!\Bigl(-i\,\tau_{\mathrm{sbd}}\,(i-n_{\mathrm{lag}})\,
   \frac{\pi}{2\,\Delta\nu_{\mathrm{sbd}}\,n_{\mathrm{lag}}}\Bigr).

The output range :math:`(x_{\mathrm{start}},x_{\mathrm{end}})` depends on
whether USB, LSB, or both are present.  Per-channel parabolic
interpolation of the peak lag gives :math:`\mathrm{sbdbox}_c`.

**Inputs:** :math:`\{V_{\mathrm{sbd},p,c,t,b}\}`, :math:`\{W_{p,c,t}\}`,
:math:`\{\Gamma^{\mathrm{phase}}_{c,t}\}`, :math:`\{\mathrm{sb}_c\}`,
:math:`\{\mathrm{BW}_c\}`, :math:`\Delta\nu_{\mathrm{sbd}}`,
:math:`\tau_{\mathrm{sbd}}`, :math:`n_{\mathrm{lag}}`.

**Outputs:** :math:`\{S^{\mathrm{xp}}_i\}` (complex),
:math:`|S^{\mathrm{xp}}_i|` (amplitude),
:math:`\arg(S^{\mathrm{xp}}_i)` (phase in degrees),
:math:`\{x^{\mathrm{xp}}_i\}` (MHz offset), :math:`\{\mathrm{sbdbox}_c\}`
(parabolic peak lag), :math:`\{N^{\mathrm{USB}}_{\mathrm{ap},c}\}`,
:math:`\{N^{\mathrm{LSB}}_{\mathrm{ap},c}\}` (USB/LSB AP counts).

*Plot consumer:* ``make_xpower_plot`` --- uses ``XPSPEC-ABS`` (blue
circles, left y-axis), ``XPSPEC-ARG`` (red circles, right y-axis in
degrees), ``XPSPEC_XAXIS`` (x-axis in MHz).

calc\_sbd\_and\_xpower\_spec
----------------------------

Combined single-pass version of ``calc_sbd`` and ``calc_xpower_spec``.
Produces the same outputs but avoids scanning the data twice.

**Inputs:** same as ``calc_sbd`` + ``calc_xpower_spec``.

**Outputs:** :math:`A_{\mathrm{SBD}}(b)`, :math:`\tau_{\mathrm{sbd},b}`,
:math:`\{S^{\mathrm{xp}}_i\}`, :math:`\{\mathrm{sbdbox}_c\}`.

Segment and Phasor Functions
=============================

calc\_segs
----------

For each AP :math:`t` and channel :math:`c` computes the segment phasor

.. math::
   z_{c,t} = V_{\mathrm{sbd},p,c,t,b_{\max}} \cdot \Gamma^{\mathrm{phase}}_{c,t}.

An *All* channel (index :math:`N_{\mathrm{chan}}`) is the
weight-normalized coherent sum over channels.

**Inputs:** :math:`\{V_{\mathrm{sbd},p,c,t,b_{\max}}\}`,
:math:`\{\Gamma^{\mathrm{phase}}_{c,t}\}`, :math:`\{W_{p,c,t}\}`,
:math:`b_{\max}`.

**Outputs:** :math:`\{z_{c,t}\}` for :math:`c=0\dots N_{\mathrm{chan}}`
(including *All*), labelled by :math:`\{\nu_c\}` and AP time.

*Plot consumer:* ``make_channel_segment_plots_alt`` --- consumes
``SEG_AMP`` (blue, left y-axis) and ``SEG_PHS`` (red, right y-axis in
degrees).  Each channel gets a subplot; the last subplot is the *All*
channel.

calc\_dr\_segs\_phase
---------------------

Merged single-pass version of ``calc_dr``, ``calc_segs``, and
``calc_phase``.  Same mathematical outputs; avoids three separate bin/AP
scans.

**Inputs:** same as the three merged functions.

**Outputs:** :math:`A_{\mathrm{DR}}(k)`, :math:`\dot{\tau}_k`,
:math:`\{z_{c,t}\}`, :math:`\{z_c^{\mathrm{fringe}}\}`,
:math:`\Phi_{\mathrm{coh}}`.

smooth\_dr\_spectrum\_tcohere
-----------------------------

Applies a circular box-car window of width
:math:`n = \lceil N_{\mathrm{drsp}}\,\Delta t_{\mathrm{ap}} /
f_{\mathrm{coh}} \rceil` (forced odd) to smooth the DR amplitude
spectrum :math:`A_{\mathrm{DR}}(k)` in place.  Only executed when the
control parameter ``t_cohere`` > 0.

**Inputs:** :math:`A_{\mathrm{DR}}(k)` (from ``calc_dr`` or
``calc_dr_segs_phase``), :math:`N_{\mathrm{drsp}}`,
:math:`\Delta t_{\mathrm{ap}}`, :math:`f_{\mathrm{coh}}`.

**Outputs:** :math:`A_{\mathrm{DR}}(k)` (smoothed, in-place).

Visibility Correction
=====================

correct\_vis
------------

Modifies the raw visibility array in-place:

.. math::
   \tilde{V}_{p,c,t,\mathrm{sp}} =
   \underbrace{\exp\!\bigl(+2\pi i \,\mathrm{sb}_c\,(\mathrm{freq}_{\mathrm{sp}}
   - \tfrac{1}{2}\mathrm{BW}_c)\,\tau_{\mathrm{sbd}}\bigr)}_{\text{SBD
   correction}} \cdot
   \underbrace{\exp\!\bigl(-2\pi i \Phi^{\mathrm{phase}}_{c,t}\bigr)}_{\text{delay
   \& delay-rate correction}} \cdot V_{p,c,t,\mathrm{sp}}

A linear d-parallactic-angle correction (``MHO_LinearDParCorrection``)
is inverted if present.  The user may request axis reduction
(--xpower-output).

**Inputs:** :math:`\{V_{p,c,t,\mathrm{sp}}\}`, :math:`\{W_{p,c,t}\}`,
:math:`\{\Gamma^{\mathrm{phase}}_{c,t}\}`, :math:`\{\mathrm{sb}_c\}`,
:math:`\{\mathrm{BW}_c\}`, :math:`\{\mathrm{freq}_{\mathrm{sp}}\}`,
:math:`\tau_{\mathrm{sbd}}`, optional d-par operator, ``xpower_output``.

**Outputs:** :math:`\{\tilde{V}_{p,c,t,\mathrm{sp}}\}` (modified
in-place).

*Plot consumer:* not plotted directly; exported to Mark4 fringe file
when ``-X`` option is set.

Residual and Diagnostic Functions
==================================

calc\_freqrms
-------------

Frequency-domain RMS residuals.  For each channel :math:`c` the
weight-normalized time-average phasor :math:`\langle z_{c,t}\rangle_t`
is compared to :math:`\Phi_{\mathrm{coh}}`.  The frequency RMS phase is

.. math::
   \mathrm{freqrms}_{\phi} = \sqrt{\frac{1}{N_{\mathrm{chan}}\!-\!2}
   \sum_{c=0}^{N_{\mathrm{chan}}-1} \bigl(\arg\langle z_{c,t}\rangle_t
   - \Phi_{\mathrm{coh}}\bigr)^2} \cdot \frac{180^\circ}{\pi}

and the frequency RMS amplitude is

.. math::
   \mathrm{freqrms}_{A} = \sqrt{\frac{1}{N_{\mathrm{chan}}}
   \sum_{c=0}^{N_{\mathrm{chan}}-1} \bigl(|\langle z_{c,t}\rangle_t|
   - \mathcal{A}\bigr)^2} \cdot \frac{100}{\mathcal{A}} \quad (\%)

The incoherent average amplitude (frequency) is the noise-bias-corrected

.. math::
   \overline{A}_{\mathrm{inc}}^{\mathrm{freq}} =
   \frac{\sum_c |\sum_t W_{p,c,t}\, z_{c,t}| / W_{\mathrm{tot}}}
   {1 + \frac{1}{2\,\mathrm{SNR}^2 / N_{\mathrm{chan}}}}

**Inputs:** :math:`\{z_{c,t}\}`, :math:`\Phi_{\mathrm{coh}}`,
:math:`\mathcal{A}` (fringe amplitude from ``quants.rst``),
:math:`W_{\mathrm{tot}}`, :math:`\mathrm{SNR}` (from ``quants.rst``).

**Outputs:** :math:`\mathrm{freqrms}_{\phi}` (deg),
:math:`\mathrm{freqrms}_{A}` (%),
:math:`\overline{A}_{\mathrm{inc}}^{\mathrm{freq}}` (bias-corrected).

*Plot consumer:* printed in the channel info table
(``make_channel_info_table``) via ``extra`` keys ``freqrms_phase`` and
``freqrms_amp``; not a separate graphical plot.

calc\_timerms
-------------

Time-domain RMS residuals.  Within each segment :math:`s` the phasors
from all channels and APs are coherently summed to give :math:`V_s`.  The
phase deviation from :math:`\Phi_{\mathrm{coh}}` is squared and weighted
by :math:`W^{\mathrm{DSB}}_s`.  The amplitude deviation from
:math:`\mathcal{A}` is similarly accumulated.  After summing over segments:

.. math::
   \mathrm{timerms}_{\phi} &= \sqrt{\frac{\sum_s W_s^{\mathrm{DSB}}
   \bigl(\arg V_s - \Phi_{\mathrm{coh}}\bigr)^2}{\sum_s W_s^{\mathrm{DSB}}}}
   \cdot \frac{180^\circ}{\pi} \\
   \mathrm{timerms}_{A} &= \sqrt{\frac{\sum_s W_s^{\mathrm{DSB}}
   \bigl(|V_s|/\!W_s^{\mathrm{DSB}} - \mathcal{A}\bigr)^2}{\sum_s W_s^{\mathrm{DSB}}}}
   \cdot \frac{100}{\mathcal{A}} \quad (\%)

The incoherent average amplitude (time) is the noise-bias-corrected
:math:`\overline{A}_{\mathrm{inc}}^{\mathrm{time}} = \frac{\sum_s |V_s|}
{\bigl(1 + N_{\mathrm{seg}}/(2\,\mathrm{SNR}^2)\bigr)\,\sum_s
W_s^{\mathrm{DSB}}}`.

Per-segment USB/LSB weight fractions :math:`w^{\mathrm{USB}}_{c,s}`,
:math:`w^{\mathrm{LSB}}_{c,s}` are also recorded for the validity plot.

**Inputs:** :math:`\{z_{c,t}\}`, :math:`\Phi_{\mathrm{coh}}`,
:math:`\mathcal{A}`, :math:`\mathrm{SNR}`, :math:`\{\mathrm{sb}_c\}`,
:math:`\{W_{p,c,t}\}`, :math:`N_{\mathrm{seg}}`,
:math:`n_{\mathrm{ap/seg}}`.

**Outputs:** :math:`\mathrm{timerms}_{\phi}` (deg),
:math:`\mathrm{timerms}_{A}` (%),
:math:`\overline{A}_{\mathrm{inc}}^{\mathrm{time}}`,
:math:`\{w^{\mathrm{USB}}_{c,s}\}`, :math:`\{w^{\mathrm{LSB}}_{c,s}\}`.

*Plot consumer:* printed in the channel info table
(``make_channel_info_table``) via ``extra`` keys ``timerms_phase`` and
``timerms_amp``.  The USB/LSB fractions feed
``make_channel_segment_validity_plots`` (green/red vertical lines for
segments above/below 95% validity).

calc\_quality\_code
-------------------

Assigns a single-character quality code ``QC`` :math:`\in
\{\text{'0'},\dots,\text{'9'}\}`.  Starting from ``'9'`` the code is
decremented for each condition where an observed RMS exceeds twice the
corresponding theoretical RMS from ``quants.rst``
(:math:`\mathrm{timerms}_{\phi} > 2\times\mathrm{th\_timerms}_{\phi}`,
etc.) or where the observed RMS exceeds an absolute threshold.  If
:math:`\mathrm{PFD} > 10^{-4}` the code is forced to ``'0'``.

**Inputs:** :math:`\mathrm{timerms}_{\phi}`, :math:`\mathrm{timerms}_{A}`,
:math:`\mathrm{freqrms}_{\phi}`, :math:`\mathrm{freqrms}_{A}` (from
above), :math:`\mathrm{th\_timerms}_{\phi}`, :math:`\mathrm{th\_timerms}_{A}`,
:math:`\mathrm{th\_freqrms}_{\phi}`, :math:`\mathrm{th\_freqrms}_{A}`
(from ``quants.rst``), :math:`\mathrm{PFD}`.

**Outputs:** ``QC`` (character ``'0'``-``'9'``).

*Plot consumer:* printed in the channel info table via key ``Quality``.

calc\_error\_code
-----------------

Assigns a single-character error code ``EC``.  ``'G'`` if any channel
amplitude :math:`|z_c^{\mathrm{fringe}}| < \mathrm{weak\_channel}
\cdot \overline{A}_{\mathrm{inc}}^{\mathrm{freq}}` and
:math:`\mathrm{SNR} > 20`.  ``'H'`` if the mean pcal amplitude is
outside :math:`[\mathrm{pc\_amp\_hcode},\,0.5]` for any channel and the
pcal mode is ``multitone``.  Default is ``' '``.

**Inputs:** :math:`\{z_c^{\mathrm{fringe}}\}`,
:math:`\overline{A}_{\mathrm{inc}}^{\mathrm{freq}}`, :math:`\mathrm{SNR}`,
``weak_channel`` (threshold, default 0.5), ``pc_amp_hcode`` (threshold,
default 0.005), pcal amplitudes, pcal modes.

**Outputs:** ``EC`` (character).

*Plot consumer:* printed in the channel info table via key
``extra/error_code``.

PCal and JSON Dump Functions
=============================

dump\_multitone\_pcmodel
------------------------

Exports multitone pcal phase, delay, and amplitude per channel for
reference and remote stations.  Populates the ``PLOT_INFO`` keys
``PCdlyRf/Rem``, ``PCPhsRf/Rem``, ``PCAmpRf/Rem`` (and ``...2``
suffixes for pseudo-Stokes I second polarization).

**Inputs:** channel axis labels (``ref/rem_mtpc_phase/delay/mag``).

**Outputs:** per-channel pcal vectors in ``plot_dict`` ``PLOT_INFO``.

*Plot consumer:* ``make_pcal_plots`` --- uses ``PCOffRf`` and
``PCOffRm`` for manual pcal offsets (green circles = ref, magenta
circles = rem, orange/purple dots = second pol for pseudo-Stokes I).
Also consumed by the channel info table.

dump\_manual\_pcmodel
---------------------

Exports manual pcal phase offsets per channel for reference and remote
stations.  Populates ``PCOffRf/Rem`` (and ``...2`` for pseudo-Stokes I).

**Inputs:** manual pcal offset data from the control file.

**Outputs:** per-channel manual pcal offsets in ``plot_dict``.

*Plot consumer:* ``make_pcal_plots`` (primary source for the pcal
:math:`\theta` curves).

DumpInfoToJSON
--------------

Main orchestrator.  Calls the computation functions in order:
``precompute_chan_metadata``, ``precompute_vr_tables``,
``calc_sbd_and_xpower_spec``, ``calc_mbd``, ``calc_dr_segs_phase``,
then post-processes the phasors to produce segment amplitudes
:math:`\{A^{\mathrm{seg}}_{c,s}\}` and phases
:math:`\{\phi^{\mathrm{seg}}_{c,s}\}`, incoherent averages, and RMS
diagnostics.  All results are serialized into the ``plot_dict`` JSON
object consumed by ``fourfit_plot.py``.

Cross-Reference: C++ Functions and Python Plots
================================================

.. list-table:: Cross-reference between C++ computation functions, JSON keys, and Python plotting routines in ``fourfit_plot.py``.
   :widths: 22 24 20 34
   :header-rows: 1

   * - C++ function
     - JSON key(s)
     - Plot function
     - Visual element
   * - ``calc_mbd``
     - ``MBD_AMP``, ``MBD_AMP_XAXIS``
     - ``make_dr_mbd_plot`` (ax2)
     - Blue line, top x-axis (:math:`\mu s`)
   * - ``calc_sbd`` / ``calc_sbd_and_xpower_spec``
     - ``SBD_AMP``, ``SBD_AMP_XAXIS``
     - ``make_sbd_dtec_plot`` (ax3)
     - Green line, bottom x-axis (:math:`\mu s`)
   * - ``calc_dr`` / ``calc_dr_segs_phase``
     - ``DLYRATE``, ``DLYRATE_XAXIS``
     - ``make_dr_mbd_plot`` (ax1)
     - Red line, bottom x-axis (ns/s)
   * - ``calc_xpower_spec`` / ``calc_sbd_and_xpower_spec``
     - ``XPSPEC-ABS``, ``XPSPEC-ARG``, ``XPSPEC_XAXIS``
     - ``make_xpower_plot`` (ax4, ax5)
     - Blue circles (amp), red circles (phase, deg)
   * - ``calc_segs`` / ``calc_dr_segs_phase``
     - ``SEG_AMP``, ``SEG_PHS``
     - ``make_channel_segment_plots_alt`` (ax6, ax6a)
     - Blue circles (amp), red circles (phase, deg)
   * - ``calc_timerms``
     - ``SEG_FRAC_USB``, ``SEG_FRAC_LSB``
     - ``make_channel_segment_validity_plots`` (ax7)
     - Green/red vertical lines (USB/LSB >= 95% or > 0%)
   * - ``dump_manual_pcmodel``
     - ``PCOffRf``, ``PCOffRm``
     - ``make_pcal_plots`` (ax8)
     - Green circles (ref), magenta circles (rem)
   * - ``calc_freqrms``, ``calc_timerms``, ``calc_quality_code``, ``calc_error_code``
     - ``PLOT_INFO/...``, ``extra/...``, ``Quality``
     - ``make_channel_info_table`` (axT)
     - Text table below plots
   * - ``correct_vis``
     - -- (in-place)
     - --
     - Exported to Mark4 fringe file (``-X`` flag)

Data-Flow Summary
=================

The top-level orchestrator ``DumpInfoToJSON`` produces the following
data flow:

1. **Precomputation:** :math:`\{\nu_c\}`, :math:`\{\mathrm{sb}_c\}`,
   :math:`\{\mathrm{BW}_c\}`, :math:`\{\Gamma_{c,t}\}`,
   :math:`\{\Gamma^{\mathrm{MBD0}}_{c,t}\}`,
   :math:`\{\Gamma^{\mathrm{phase}}_{c,t}\}`.

2. **Merged pass 1:** :math:`A_{\mathrm{SBD}}(b)` +
   :math:`\{S^{\mathrm{xp}}_i\}` + :math:`\{\mathrm{sbdbox}_c\}`.

3. **MBD pass:** :math:`A_{\mathrm{MBD}}(k)`.

4. **Merged pass 2:** :math:`A_{\mathrm{DR}}(k)` +
   :math:`\{z_{c,t}\}` + :math:`\{z_c^{\mathrm{fringe}}\}` +
   :math:`\Phi_{\mathrm{coh}}`.

5. **Post-processing:** segment averages
   :math:`\{A^{\mathrm{seg}}_{c,s}\}`,
   :math:`\{\phi^{\mathrm{seg}}_{c,s}\}`; incoherent averages
   :math:`\overline{A}_{\mathrm{inc}}^{\mathrm{freq}}`,
   :math:`\overline{A}_{\mathrm{inc}}^{\mathrm{time}}`; RMS diagnostics;
   quality/error codes.

6. **Visibility correction:**
   :math:`\{\tilde{V}_{p,c,t,\mathrm{sp}}\}` (optional export).

The theoretical RMS values
(:math:`\mathrm{th\_timerms}_{\phi}`, :math:`\mathrm{th\_timerms}_{A}`,
:math:`\mathrm{th\_freqrms}_{\phi}`, :math:`\mathrm{th\_freqrms}_{A}`)
are computed by ``MHO_BasicFringeInfo`` functions (see ``quants.rst``)
and are used by ``calc_quality_code`` for comparison.
