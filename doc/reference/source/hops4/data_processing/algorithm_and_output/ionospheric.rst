.. _ionospheric:

Ionospheric Fringe Fitting: Solving for Differential TEC
--------------------------------------------------------

The ``MHO_IonosphericFringeFitter`` class extends the standard three-dimensional
fringe-fitting algorithm by adding a fourth search dimension: the differential
Total Electron Content (dTEC) between the reference and remote stations.
This addresses the dispersive ionospheric phase, which scales as :math:`\nu^{-1}`
with observing frequency, and can severely degrade the coherent
combination of wide-band visibility data when left uncorrected.

Physical Background
~~~~~~~~~~~~~~~~~~~

The ionosphere is a plasma layer in the Earth's upper atmosphere that introduces
a frequency-dependent phase delay to radio signals. This phase delay depends
on the integrated total electron content (TEC) along the propagation path.
For a particluar station *A* and source *S* the instantaneous
line-of-sight (or slant) TEC (STEC) is given by:

.. math::

   \mathrm{STEC}_{A,S}(t) = \int_{0}^{\infty}
       n_e(r,\, \theta_S,\, \phi_S,\; t)
       \,\mathrm{d}r

The differential TEC (dTEC) for the baseline AB is the difference in
the integrated electron column density
along the line of sight from each of the two stations to the source:

.. math::

    \mathrm{dTEC} = \mathrm{STEC}_B - \mathrm{STEC}_A

A non-zero dTEC has the effect of accumulating a frequency dependent phase over
the VLBI visibilities, which to first order (and ignoring time dependence) is approximately [Cappallo2016]_:

.. math::

   \theta_{\mathrm{ion}} = \frac{\kappa_{\mathrm{ion}} \cdot \langle \mathrm{dTEC} \rangle}
   {10^6 \cdot \nu_{c}}

where:


.. list-table::
   :header-rows: 1
   :widths: 15 50 20

   * - Variable
     - Description
     - Units
   * - :math:`\kappa_{\mathrm{ion}} = -8.448 \times 10^9`
     - Ionospheric constant
     - :math:`\mathrm{rad \cdot Hz \cdot TECu}^{-1}`
   * - :math:`\mathrm{dTEC}`
     - Differential TEC
     - :math:`\mathrm{TECu}` [#]_
   * - :math:`\nu_c`
     - Channel center frequency
     - :math:`\mathrm{MHz}`

.. [#] Where :math:`1~\mathrm{TECu} = 10^{16}~\mathrm{e^-/m^2}`

Because the ionospheric effect scales as :math:`1/\nu`, lower-frequency channels
experience larger phase rotations. Over a multi-GHz bandwidth, this differential
phase can smear the fringe amplitude across the band, making it indistinguishable
from a multi-band delay error if uncorrected. Removing the effect of the dTEC is
crucial for VGOS geodetic observations.

Ionospheric Phase Correction Operator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The class ``MHO_IonosphericPhaseCorrection`` applies a dTEC correction
in-place on the visibility tensor. For each channel, the operator:

1. Retrieves the channel's sky frequency :math:`\nu_c`, bandwidth :math:`B_c`, and net
   sideband (USB/LSB/DSB).
2. Computes the channel's center frequency:
   :math:`\nu_{c,\mathrm{center}} = 0.5 \cdot (\nu_{\mathrm{lower}} + \nu_{\mathrm{upper}})`.
3. Calculates the ionospheric phase rotation using the equation above.
4. Multiplies the entire visibility slice for that channel by the complex
   phasor :math:`\Psi_{\mathrm{ion}}(c)`, given by:

.. math:: \Psi_{\mathrm{ion}}(c) = \exp(i \cdot \theta_{\mathrm{ion}}(c))

5. Records the applied phase (in degrees) as channel metadata under the key
   ``dtec_phase_deg``.

This correction is *additive* in phase space but *multiplicative*
in the complex visibility domain. Reversing a previously applied correction
is achieved by re-applying the operator with the *negated* dTEC value
(this is used extensively during the dTEC search).

Two-Stage Search Strategy
~~~~~~~~~~~~~~~~~~~~~~~~~

The ionospheric fringe fitter provides two search strategies, selected by the
control-file flag ``ion_smooth``:

- **Standard search** (``rjc_ion_search``): A four-level
  hierarchical search (coarse :math:`\to` medium :math:`\to` fine :math:`\to` final) with
  parabolic interpolation at the final stage.
- **Smoothed search** (``ion_search_smooth``): A three-level
  hierarchical search (coarse :math:`\to` fine :math:`\to` final) that applies a
  cosine-window smoothing and fourfold interpolation to the coarse points
  before the fine search. This is more robust when the coarse sampling is
  sparse relative to the width of the TEC response curve.

Both strategies follow the same outer-loop structure described below.

Search Window and Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The search is configured via control-file parameters:

- ``ion_win`` -- two-element search window :math:`[w_0,\, w_1]` in TEC
  units (default :math:`[0,\,0]`, i.e., no search).
- ``ion_npts`` -- number of coarse grid points (default :math:`1`, meaning
  a fixed-fit with a priori ionospheric values).
- ``ion_smooth`` -- boolean flag to enable cosine smoothing
  (default ``false``).

When ``ion_npts = 1``, the fitter reads a priori ionospheric values from
the station entries in the control file (keyword ``ionosphere``, default value is 0)
and fixes dTEC to:

.. math::

   \mathrm{dTEC} = \mathrm{TEC}_{\mathrm{remote}} -
   \mathrm{TEC}_{\mathrm{reference}}

applies a single ionospheric phase correction and then performs a single fringe fit.

Hierarchical Search Levels
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Standard (Four-Level) Search**

The standard search proceeds through four levels:

1. **Coarse** (level 0): A uniform grid of ``ion_npts`` points
   spanning :math:`[w_0,\, w_1]`. For each dTEC trial point, the full three-dimensional
   fringe search (SBD, DR, MBD) is executed. The maximum amplitude across the
   coarse grid determines the center of the medium search.

2. **Medium** (level 1): :math:`N_{\mathrm{med}} = 12` points with a step
   size of :math:`2.0` TECu, centered on the coarse maximum. Edge cases (peak at
   boundary) are handled by shifting the center inward.

3. **Fine** (level 2): :math:`N_{\mathrm{fine}} = 12` points with a step
   size of :math:`0.4` TECu, centered on the medium maximum.

4. **Final** (level 3): The three points around the fine maximum are
   fitted with a parabola (using ``MHO_MathUtilities::parabola``) to
   extract the sub-grid dTEC peak location. The parabola coefficients :math:`q` and
   the interpolated peak :math:`(x_{\max},\, A_{\max})` are returned.

**Smoothed (Three-Level) Search**

The smoothed search replaces the medium level with a smoothing step:

1. **Coarse** (level 0): Identical to the standard coarse search.

2. **Smoothing and fourfold interpolation**: The coarse amplitude
   array is up-sampled by a factor of 4 (inserting zeros between points) and
   convolved with a half-cycle cosine kernel of width:

   .. math:: n_s = \max\left(1,\; \left\lceil \frac{36}{\delta_{\mathrm{TEC}}} \right\rceil\right)

   (clipped to the array length, forced odd). The half-cosine kernel has a
   half-power width of approximately :math:`3` TECu, matched to the expected
   correlation width of the TEC response for :math:`3`--:math:`10` GHz frequency
   distributions. The interpolated maximum determines the center of the
   fine search.

3. **Fine** (level 1): :math:`N_{\mathrm{fine\_smooth}} = 24` points with
   a step size of :math:`0.4` TECu, centered on the smoothed maximum.

4. **Final** (level 2): Identical parabolic interpolation to the
   standard search.

Inner Loop: Visibility Modification per dTEC Trial
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For each dTEC trial value :math:`\delta_{\mathrm{trial}}` in the outer loop, the
algorithm modifies the visibilities *in-place* as follows:

1. **Undo** the previous dTEC correction:

   .. code-block:: text

      iono.SetDifferentialTEC(delta_prev);
      iono.Execute();

   This restores the visibilities to their uncorrected state.

2. **Apply** the current dTEC correction:

   .. code-block:: text

      iono.SetDifferentialTEC(-delta_trial);
      iono.Execute();

   The application of the ionospheric effect on the visibilities is done *in-place*
   in order to avoid doubling the memory requirements. The first step is required to reset
   the visibilities back to their original state, while the negation in the second step
   arises because the operator *corrects* the data by removing the ionospheric effect,
   which requires applying the *opposite* sense phase rotation as would have been
   accumulated by the net (differential) propagation through the ionosphere.

3. Execute the full coarse fringe search (``coarse_fringe_search``)
   followed by fine peak interpolation (``interpolate_peak``).

4. Store the peak amplitude ``famp`` for this dTEC trial.

After the first dTEC trial, the algorithm caches the full SBD search window
and, if the approximate SNR exceeds :math:`15`, narrows the SBD window to
:math:`\pm 1` bin around the fitted SBD to accelerate subsequent trials.
This can greatly accelerate ionospheric fringe fitting in the strong (high-SNR) fringe case.

Output and Solution
~~~~~~~~~~~~~~~~~~~

Upon completion of the hierarchical search, the dTEC value corresponding to the maximum fringe amplitude
is stored in the parameter store as ``/fringe/ion_diff``. The full dTEC vs. fringe amplitude
scan (sorted by dTEC value) is also stored as two parallel arrays:
``/fringe/dtec_array`` and ``/fringe/dtec_amp_array``.

The standard solution quantities (SNR, delays, errors, residual phase, PFD,
quality code) are then computed by
``MHO_BasicFringeUtilities::calculate_fringe_solution_info``,
using the visibilities with the best dTEC correction already applied. When
ionospheric correction is active, the MBD error is recomputed from the
ionospheric covariance matrix to account for the additional uncertainty
introduced by the dTEC fit.
