MHO\_PolProductSummation
========================

Purpose
-------
``MHO_PolProductSummation`` coherently sums multiple polarization products
(e.g., ``XX`` + ``YY`` -> pseudo-Stokes-I) into a single polarization-product
entry along the polarization-product axis of visibility data. Each polarization
product may be pre-multiplied by a parallactic-angle-dependent factor
(see ``MHO_LinearDParCorrection`` or ``MHO_CircularFieldRotationCorrection``) before
summation, and the resulting sum is normalized by the total absolute prefactor
magnitude.

Control File Trigger
--------------------
- **Keyword:** ``polproduct_sum``
- **Category:** prefit
- **Priority:** 9.99
- **Parameters:** None (the polarization-product set and sum label are derived internally from the baseline configuration).

Input Data
----------
The operator works on ``visibility_type`` containers (4D axis pack: polarization-product x channel x time x frequency). It also requires a pointer to the associated ``weight_type`` container. Station coordinate data for both reference and remote stations, along with their parallactic angles, are used to compute parallactic-angle-dependent pre-factors.

Algorithm
---------
``MHO_PolProductSummation`` uses two internal ``MHO_Reducer`` operators: one for the visibility array (``fReducer``) and one for the weight array (``fWReducer``). Both reducers are configured to sum along the ``POLPROD_AXIS``.

**Initialization (``InitializeInPlace`` / ``InitializeOutOfPlace``):**

1. Configure ``fReducer`` to reduce the visibility container's polarization-product axis using the ``MHO_CompoundSum`` functor (element-wise addition).
2. Configure ``fWReducer`` to reduce the weight container's polarization-product axis using the same functor.
3. Both reducers resize their output workspaces so that the polarization-product dimension has size 1, while all other axes retain their original size.

**Execution (``ExecuteInPlace`` / ``ExecuteOutOfPlace``):**

1. **Pre-multiply** each polarization product by its complex pre-factor, normalized by the sum of absolute pre-factors (see below).
2. Execute the visibility reducer: the reducer collapses all polarization products into a single entry via element-wise complex addition across all other axes (channel, time, frequency).
3. Execute the weight reducer: similarly collapses all weight entries into a single summed weight.
4. **Fix labels:** set the single remaining polarization-product label on the output axis to the configured sum label (e.g., ``I`` for pseudo-Stokes-I).
5. Insert the tag ``n_summed_polprod`` into the weight container, recording the number of polarization products that were summed.

**Pre-factor calculation (``PreMultiply`` and ``GetPrefactor``):**

The pre-factor depends on the difference in parallactic angle between the reference and remote stations:

.. math::

    \Delta\psi = (\psi_{\mathrm{rem}} - \psi_{\mathrm{ref}}) \cdot \frac{\pi}{180}

where :math:`\psi_{\mathrm{ref}}` and :math:`\psi_{\mathrm{rem}}` are the parallactic angles in degrees.

For linear polarization products, when more than one product is summed (``fPolProductSet.size()`` > 1), the pre-factors are:

.. math::

    \begin{aligned}
    \text{XX} &: \cos(\Delta\psi) \\
    \text{YY} &: \cos(\Delta\psi) \\
    \text{XY} &: \sin(-\Delta\psi) \\
    \text{YX} &: \sin(\Delta\psi)
    \end{aligned}

When only a single polarization product is present, the pre-factor uses the sign of the trigonometric value rather than the value itself:

.. math::

    \text{factor} = \mathrm{signum}(\text{trig value})

For circular polarization products (RR, LL, RL, LR), the pre-factor is always 1.0 (handled by a separate ``MHO_CircularFieldRotationCorrection`` operator for full correction).

Within ``PreMultiply``, the operator computes the sum of absolute pre-factors:

.. math::

    S = \sum_{k} |c_k|

For pseudo-Stokes-I mode (sum label ``I``, typically 4 polarization products), :math:`S` is explicitly set to 2.0 (the number of polarizations). Each visibility slice is then scaled:

.. math::

    V[i] \gets V[i] \cdot \frac{c_i}{S}

**Weight treatment:**

The weight reducer sums all weights from the contributing polarization products.

Effect on Data
--------------
The operator reduces the polarization-product axis from N entries to a single
entry containing the coherently summed visibility. Each input polarization
product is scaled by a parallactic-angle-dependent pre-factor before summation.
The corresponding weight axis is also reduced to a single entry (the sum of
the individual product weights). The polarization-product label is replaced
by the configured sum label (e.g., ``I``). All other axes (channel, time,
frequency) are unaffected. The number of summed products is recorded as
the ``n_summed_polprod`` tag on the weight container.
