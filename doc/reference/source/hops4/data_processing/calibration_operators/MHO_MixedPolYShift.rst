MHO\_MixedPolYShift
===================

Purpose
-------
``MHO_MixedPolYShift`` applies a static -90 degree phase offset to the
Y-polarization component of mixed linear/circular polarization products
(e.g., ``RY`` or ``YR``). This correction ensures that mixed-polarization
cross-products (``XR+YR`` or ``RX+RY``) sum coherently. This operator is
primarily intended for use duringr mixed VGOS-SX observations.

Control File Trigger
--------------------
- **Keyword:** ``mixed_pol_yshift90``
- **Category:** calibration
- **Priority:** 3.5

.. list-table:: Parameters for ``mixed_pol_yshift90``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - mixed_pol_yshift90
     - boolean
     - When ``true``, enables the Y-pol -90 degree phase shift for mixed linear/circular polarization products; when ``false``, the operator is not created.

Input Data
----------
This operator acts on the ``visibility_type`` container in-place.

Algorithm
---------
The operator has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.
The phase offset is fixed at -90 degrees (configurable via ``SetPhaseOffset``,
though the builder always uses -90 degrees).

**Execution (``ExecuteInPlace``):**

1. Iterate over both stations: reference (``st_idx = 0``) and remote (``st_idx = 1``).
2. For each station, retrieve the station identifier from the visibility container tags.
3. Iterate over all polarization products on the POLPROD_AXIS:

   a. Check if the polarization product is a mixed linear/circular product by calling ``IsMixedLinCirc``, which returns true when the product contains both a circular label (``R`` or ``L``) and a linear label (``X`` or ``Y``).
   b. Check if the ``IsApplicable`` condition is met: the product must contain ``Y`` at the position corresponding to the current station index (index 0 for reference, index 1 for remote).
   c. If applicable, compute the base phasor:

      .. math::

         \Phi_{\rm shift} = \exp\!\left(i \cdot \phi_Y \cdot \frac{\pi}{180}\right) = \exp\!\left(-i \cdot \frac{\pi}{2}\right)

      where :math:`\phi_Y` (the ``fYPolPhaseOffset`` member) defaults to -90.0 degrees.
   d. For the reference station (``st_idx = 0``), complex-conjugate the phasor:

      .. math::

         \Phi_{\rm shift,\,ref} = \overline{\Phi_{\rm shift}} = \exp\!\left(i \cdot \frac{\pi}{2}\right)

   e. For each frequency channel, determine the net sideband from the channel axis label ``net_sideband``. For LSB channels (``net_sideband`` = ``L``), conjugate the phasor again to account for the sideband sign flip:

      .. math::

         \Phi_{\rm LSB} = \overline{\Phi_{\rm shift}}

   f. Apply the final phasor by multiplying the visibility sub-view for that (pol-product, channel) pair:

      .. math::

         V[p, ch, :, :] \leftarrow V[p, ch, :, :] \cdot \Phi_{\rm applied}

The net effect depends on station and sideband:

- Remote + USB: multiply by :math:`\exp(-i\pi/2) = -i`
- Remote + LSB: multiply by :math:`\exp(+i\pi/2) = +i` (conjugated for LSB)
- Reference + USB: multiply by :math:`\exp(+i\pi/2) = +i` (conjugated for reference)
- Reference + LSB: multiply by :math:`\exp(-i\pi/2) = -i` (conjugated twice: reference + LSB)

Effect on Data
--------------
For each mixed linear/circular polarization product containing a ``Y``
polarization, the operator multiplies the entire channel's visibility data
by a +/-90 degree phase phasor. The sign of the phase rotation depends on
both which station carries the Y-polarization (reference vs remote) and the
channel's net sideband (USB vs LSB).
