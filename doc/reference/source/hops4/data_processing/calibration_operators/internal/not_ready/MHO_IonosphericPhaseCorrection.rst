MHO\_IonosphericPhaseCorrection
=================================

Purpose
-------

This operator applies a differential ionospheric phase correction to visibility
data on a per-channel basis. The Earth's ionosphere introduces a
frequency-dependent phase delay that varies with the Total Electron Content
(TEC) along the line of sight. When the reference and remote stations view a
source from different directions, they experience different TEC, producing a
differential phase rotation across the baseline. This operator corrects that
rotation by phase-shifting each spectral channel according to the differential
TEC (dTEC) between the two stations.

Control File Trigger
--------------------

This operator has no dedicated control-file keyword or builder.
It is used internally by ``MHO_IonosphericFringeFitter`` as part of the
ionospheric fringe-fitting search. The dTEC value is set programmatically
via the ``SetDifferentialTEC()`` method.

The ionospheric fringe-fitting workflow is controlled through the following
fit parameters in the control file:

- ``ion_win`` -- two-element search window :math:`[w_0,\, w_1]` in TEC units (TECu)
- ``ion_npts`` -- number of trial points in the dTEC search grid
- ``ion_smooth`` -- smoothing flag for the search
- ``ionosphere`` -- real-valued station parameter (per station, in TECu); when ``ion_npts = 1``, the differential value :math:`\text{ionosphere}_{\rm remote} - \text{ionosphere}_{\rm reference}` is used as a fixed dTEC

Input Data
----------

The operator works in-place on a ``visibility_type`` container. The container carries a channel axis labeled by sky frequency (in MHz), with per-channel metadata keys:

- ``net_sideband`` -- string indicating ``L`` (lower sideband) or ``U`` (upper sideband)
- ``bandwidth`` -- channel bandwidth in MHz

Both keys are required for every channel; the operator fails with an error message if either is missing.

Algorithm
---------

The operator iterates over every polarization product and every spectral channel. For each channel, it determines the upper and lower frequency limits from the sky frequency, bandwidth, and net sideband, and computes the channel's center frequency:

.. math::

   f_{\rm center} = \tfrac{1}{2}\,(f_{\rm lower} + f_{\rm upper})

The ionospheric phase rotation (in radians) is then computed as:

.. math::

   \theta_{\rm ion} = \frac{\kappa_{\rm ion} \cdot \mathrm{dTEC}}{10^6 \cdot f_{\rm center}}

where :math:`\kappa_{\rm ion} = -8.448 \times 10^9` is the ionospheric constant (in units of :math:`\mathrm{rad \cdot Hz \cdot TECu^{-1}}`), :math:`\mathrm{dTEC}` is the differential TEC in TECu, and :math:`f_{\rm center}` is the channel center frequency in MHz. The factor :math:`10^6` converts MHz to Hz.

From this phase, the operator constructs a complex phasor:

.. math::

   P_{\rm ion} = \exp\!\bigl(j \cdot \theta_{\rm ion}\bigr)

where :math:`j` is the imaginary unit. This phasor is applied to the visibility sub-view for that polarization product and channel.

After applying the correction, the operator records the ionospheric phase
(in degrees) as metadata on the channel axis under the key ``dtec_phase_deg``:

.. math::

   \theta_{\rm ion}^{\rm (deg)} = \theta_{\rm ion} \cdot \frac{180}{\pi}

Effect on Data
--------------

This operator modifies the input visibility container in-place. Each visibility
sub-view (indexed by polarization product and channel) is multiplied by a
complex phasor :math:`\exp(j \cdot \theta_{\rm ion})` that compensates for the
differential ionospheric phase delay at that channel's center frequency.
Additionally, the channel axis metadata is updated with a ``dtec_phase_deg``
tag recording the phase rotation in degrees. The sign convention is such that a
positive dTEC (more TEC at the remote station) introduces a negative phase
rotation, which the operator corrects by applying a positive compensating
phasor. When the ionospheric fringe fitter needs to undo a previously applied
correction, it re-applies this operator with the negated dTEC value.
