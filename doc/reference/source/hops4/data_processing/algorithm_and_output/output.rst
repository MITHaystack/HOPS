.. _output:

Solution Quantities and Diagnostics
-----------------------------------

After obtaining the refined fringe solution, the algorithm computes derived
quantities: the signal-to-noise ratio (SNR), residual phase, formal error estimates,
probability of false detection, and other quality indicators. These are given as follows.

Signal-to-Noise Ratio
~~~~~~~~~~~~~~~~~~~~~
The signal-to-noise ratio (for a broadband continuum source) is given by:

.. math::

   \mathrm{SNR} = A_{\mathrm{fringe}} \; f_2 \, f_e \, f_w \; \kappa_{\mathrm{BW}}
   \sqrt{N_{\mathrm{pol}} \cdot W_{\mathrm{tot}} \cdot \frac{\Delta t}{T_{\mathrm{samp}}}} \;

where :math:`N_{\mathrm{pol}}` is the effective number of polarizations (1 or 2),
:math:`W_{\mathrm{tot}}` is the total summed weights (proportional to used bandwidth) across all channels and APs,
:math:`\Delta t` is the integration time per AP, and :math:`T_{\mathrm{samp}}` is
the sample period. The remaining factors are hard-coded correction constants, :math:`f_2 = 0.881` (2-bit sampling [#f1]_),
:math:`f_e = 0.970` (DiFX/Mark4 empirical normalization factor), and :math:`f_w = 10^{-4}`
(amplitude unit conversion from "Whitneys"), and :math:`\kappa_{\mathrm{BW}}` is a bandwidth
correction factor accounting for notched or passband-reduced channels:

.. [#f1] This hard-coded factor needs to be modified for other bit-depth sampling schemes.

.. math::

   \kappa_{\mathrm{BW}} = \frac{1}{\sqrt{n_{\mathrm{summed\_polprod}}}} \;
   \sqrt{\frac{\sum\limits_{c,a} f_{c,a}}{\sum\limits_{c,a} 1}}

where :math:`f_{c,a}` is the used bandwidth fraction for channel *c* at AP *a*,
and :math:`n_{\mathrm{summed\_polprod}}` is the number of summed polarization products
from the weight object.

Residual Phase
~~~~~~~~~~~~~~

The residual phase is the argument of the coherently summed (weighted), counter-rotated
visibility at the peak SBD lag. Note that this is done over the visibility array after
the polarization product has been selected or a summation over pol-products has been performed :math:`(p'=0)`, thus:

.. math::

   Z_{\mathrm{res}} = \sum\limits_{c,a} W[p',c,a,0] \;
   \mathcal{S}'[p',c,a,\ell^*] \;
   \Psi(c, a, \tau_{\mathrm{MBD}}, \dot{\tau}_{\mathrm{DR}})

.. math:: \phi_{\mathrm{res}} = \arg(Z_{\mathrm{res}}) \quad \text{(radians)}

The residual phase in degrees is :math:`\phi_{\mathrm{res}}^{\circ} =
\mathrm{fmod}(\phi_{\mathrm{res}} \cdot 180/\pi,\; 360)`.

The term :math:`\Psi(c, a, \tau_{\mathrm{MBD}}, \dot{\tau}_{\mathrm{DR}})` is the fringe-rotation
correction factor, implemented by ``MHO_FringeRotation::vrot``.
It is a complex phasor that de-rotates each channel/AP sample by the phase accumulated due
to the fitted multi-band delay and delay rate, and is given by:

.. math::

   \Psi(c, a, \tau_{\mathrm{mbd}}, \dot{\tau}_{\mathrm{dr}})
   = \exp\!\Bigl(-2\pi i \bigl[
       \nu_c \, \dot{\tau}_{\mathrm{dr}} \, \Delta t_a
       + \tau_{\mathrm{mbd}} \, (\nu_c - \nu_{\mathrm{ref}})
       + \delta_{\mathrm{SB}}(\tau_{\mathrm{mbd}}, c)
   \bigr]\Bigr)

where:

- :math:`\nu_c` is the sky frequency of channel *c* (MHz)
- :math:`\dot{\tau}_{\mathrm{dr}}` is the fitted delay rate (:math:`\mathrm{s}^{-1}`)
- :math:`\Delta t_a = t_a + \Delta t/2 - t_{\mathrm{FRT}}` is the time offset
  of AP *a* from the fourfit reference time (FRT), in seconds
- :math:`\tau_{\mathrm{mbd}}` is the fitted multi-band delay (us)
- :math:`\nu_{\mathrm{ref}}` is the reference frequency (MHz)
- :math:`\delta_{\mathrm{SB}}` is a sideband-dependent correction term that accounts
  for the zero-padding offset in the SBD domain (see ``Math/src/MHO_FringeRotation.cc``)

Integration Time
~~~~~~~~~~~~~~~~

The integration time is a measure of what portion of the scan's data was actually used
by the correlator and the fringe-fitter, and is given by:

.. math::

   t_{\mathrm{int}} = \frac{W_{\mathrm{tot}} \cdot \Delta t}
   {N_{\mathrm{pol}} \cdot N_c}

Total Delays
~~~~~~~~~~~~

The a priori delay :math:`\tau_0` and a priori rate :math:`\dot{\tau}_0` are computed
from the geometric delay model (station coordinates, source position,
Earth orientation parameters). Total delays are:

.. math::

   \tau_{\mathrm{total}}^{\mathrm{SBD}} &= \tau_0 + \tau_{\mathrm{SBD}} \\
   \tau_{\mathrm{total}}^{\mathrm{MBD}} &= \tau_0 + \tau_{\mathrm{MBD}}
       + \Delta_{\mathrm{ambig}} \\
   \dot{\tau}_{\mathrm{total}} &= \dot{\tau}_0 + \dot{\tau}_{\mathrm{DR}}

where :math:`\Delta_{\mathrm{ambig}}` is an ambiguity correction applied when
``mbd_anchor = "sbd"``:

.. math::

   \Delta_{\mathrm{ambig}} = \tau_{\mathrm{amb}} \cdot
   \mathrm{round}\!\left(\frac{\tau_{\mathrm{total}}^{\mathrm{SBD}}
   - \tau_{\mathrm{total}}^{\mathrm{MBD}}}{\tau_{\mathrm{amb}}}\right)

and :math:`\tau_{\mathrm{amb}}` is the MBD delay ambiguity (proportional to the inverse of the MBD grid spacing).

Error Estimates
~~~~~~~~~~~~~~~

The formal error estimates are:

.. math::

   \sigma_{\mathrm{MBD}} &= \frac{1}{2\pi \, \Delta\nu_{\mathrm{spread}} \cdot \mathrm{SNR}} \\[2ex]
   \sigma_{\mathrm{SBD}} &= \frac{4\sqrt{12} \, \delta_{\mathrm{SBD}}}
   {2\pi \, \mathrm{SNR} \, (2 - |\overline{S}|)} \\[2ex]
   \sigma_{\mathrm{DR}} &= \frac{\sqrt{12}}{2\pi \, \mathrm{SNR} \cdot \nu_{\mathrm{ref}}
   \cdot N_a \cdot \Delta t} \\[2ex]
   \sigma_{\phi} &= \frac{180}{\pi} \cdot \frac{\sqrt{1 + 3\overline{S}^2}}{\mathrm{SNR}}
   \quad \text{(degrees)}

where :math:`\overline{S}` is the SBD averaging factor from ``calculate_sbavg``,
:math:`\Delta\nu_{\mathrm{spread}}` is the frequency spread across channels,
:math:`\delta_{\mathrm{SBD}}` is the SBD separation, :math:`\nu_{\mathrm{ref}}` is
the reference frequency, :math:`N_a` is the number of APs, and :math:`\Delta t` is
the AP period.

When ionospheric phase correction is active, the MBD error is recomputed
from the ionospheric covariance matrix.

Probability of False Detection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Another quality measure for a fringe is given by the probability of false detection:

.. math::

   \mathrm{PFD} = 1 - \left(1 - \exp\left(-\frac{\mathrm{SNR}^2}{2}\right)\right)^{N_{\mathrm{search}}}

When this evaluates to less than 0.01, the small-PFD approximation is used instead:

.. math::

   \mathrm{PFD} \approx N_{\mathrm{search}} \cdot \exp\left(-\frac{\mathrm{SNR}^2}{2}\right)

where :math:`N_{\mathrm{search}}` is the total number of independent points
searched in the coarse grid.
