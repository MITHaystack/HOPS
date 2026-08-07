MHO\_LinearDParCorrection
=========================

Purpose
-------
``MHO_LinearDParCorrection`` applies a linear delta-parallactic-angle correction
to visibility data. The operator computes a purely real scaling prefactor
for each polarization product based on the parallactic angle difference
between the reference and remote stations, and multiplies each polarization
product's visibility data by that prefactor in-place. This operation is used when
constructing the pseudo-Stoke-I product (``-P I``) for VGOS observations.

Control File Trigger
--------------------
- **Keyword:** ``dpar_corr``
- **Category:** calibration
- **Priority:** 3.99

This keyword carries no JSON parameters; the parallactic angle values are
obtained from the parameter store at construction time.

Input Data
----------
This operator acts on the ``visibility_type`` container.

Algorithm
---------
``MHO_LinearDParCorrection`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

The builder (``MHO_LinearDParCorrectionBuilder``) retrieves the polarization product set, the reference station parallactic angle, and the remote station parallactic angle from the parameter store, and configures the operator accordingly.

**Execution (``ExecuteInPlace``):**

1. Retrieve the polarization-product axis from the visibility container.
2. For each polarization product, compute a prefactor from the parallactic angle difference:

   a. Compute the delta-parallactic angle in radians:

      .. math::

         \Delta p = (p_{\rm rem} - p_{\rm ref}) \cdot \pi / 180

   b. Determine the prefactor based on the polarization-product label:

      - ``XX``: :math:`\mathrm{signum}(\cos(\Delta p))`
      - ``YY``: :math:`\mathrm{signum}(\cos(\Delta p))`
      - ``YX``: :math:`\mathrm{signum}(\sin(\Delta p))`
      - ``XY``: :math:`\mathrm{signum}(\sin(-\Delta p))`
      - Any label not in the configured product set: 0

   c. Multiply every element of the visibility data for that polarization product by the prefactor.


Effect on Data
--------------
Each polarization product's visibility data is scaled by a purely real
factor (+1 or -1, or 0 if the label is unrecognized). The factor depends on
the sign of either the cosine or sine of the parallactic angle difference,
depending on the polarization product. The operator does not modify channel,
spectral, or time axes.
