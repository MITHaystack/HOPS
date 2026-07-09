MHO\_LSBOffset
==============

Purpose
-------
``MHO_LSBOffset`` applies a phase offset to the lower sideband (LSB) channels
of double-sideband (DSB) channel pairs. The operator constructs a complex phasor
from a user-supplied phase offset value (in degrees) and multiplies the
visibility data of LSB channels by that phasor. The correction is applied
independently for the reference and remote stations, with conjugation
applied to the phasor that is applied to the remote station.

Control File Trigger
--------------------
- **Keyword:** ``lsb_offset``
- **Category:** calibration
- **Priority:** 3.4

.. list-table:: Parameters for ``lsb_offset``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - value
     - real
     - The LSB phase offset in degrees.

Input Data
----------
This operator acts on the ``visibility_type`` container.

Algorithm
---------
``MHO_LSBOffset`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

The builder (``MHO_LSBOffsetBuilder``) retrieves the phase offset value from the ``value`` attribute and the target station identifiers from the control file, then configures the operator.

**Execution (``ExecuteInPlace``):**

1. Iterate over the reference (index 0) and remote (index 1) stations.
2. For each station, check applicability via ``IsApplicable``:

   a. Retrieve the station's Mark4 ID and 2-character station code from the visibility container.
   b. Match against the configured station identities. A 1-character identity is matched against the Mark4 ID (with ``?`` as wildcard); a 2-character identity is matched against the station code (with ``??`` as wildcard).

3. If applicable, construct the LSB phasor:

   .. math::

      \Phi = \exp\!\left(i \cdot \phi_{\rm lsb} \cdot \pi/180\right)

   where :math:`\phi_{\rm lsb}` is the phase offset in degrees and :math:`\pi/180` is the degrees-to-radians conversion. For the remote station (``st_idx = 1``), the phasor is conjugated: ``Phi_rem = conj(Phi)``.

4. Iterate over all polarization products and channels:

   a. For each channel, check for the presence of both the ``dsb_partner`` and ``net_sideband`` labels.
   b. If ``dsb_partner`` is present and ``net_sideband`` equals ``L`` (lower sideband), multiply all visibility data for that (polarization-product, channel) sub-view by ``Phi``.


Effect on Data
--------------
For each station that matches the configured station identities, the
visibility data of every lower-sideband channel within a double-sideband
pair is multiplied by the phasor ``exp(i * phi_lsb * pi/180)``. The reference
station receives the phasor directly; the remote station receives the
conjugated phasor. Only channels tagged with both ``dsb_partner`` and
``net_sideband = L`` are modified. Upper-sideband channels and non-DSB
channels are untouched.
