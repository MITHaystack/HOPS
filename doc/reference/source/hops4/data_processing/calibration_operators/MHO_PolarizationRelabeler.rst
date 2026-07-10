MHO\_PolarizationRelabeler and MHO\_PolarizationProductRelabeler
================================================================

Purpose
-------
``MHO_PolarizationRelabeler`` swaps single-character polarization
labels (e.g., ``X`` <-> ``Y`` or ``R`` <-> ``L``) on the polarization
axis of multitone pcal data containers. ``MHO_PolarizationProductRelabeler``
performs an analogous swap on two-character polarization-product
labels (e.g., ``XX`` -> ``YY``) along the polarization-product axis of
visibility and weight containers. Both operators are scoped to specific
stations and are used to correct antenna feed labeling in both phase-calibration and
visibility data.

Control File Trigger
--------------------
- **Keyword:** ``swap_pol_labels``
- **Category:** labeling
- **Priority:** 0.1

.. list-table:: Parameters for ``swap_pol_labels``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - pol1
     - string
     - First polarization label (single character, e.g. ``X``).
   * - pol2
     - string
     - Second polarization label (single character, e.g. ``Y``).

The keyword may be scoped to one or more stations via the standard ``station``
qualifier. Station targeting accepts either a 1-character Mark4 station ID, a
2-character station code, or the wildcards ``?`` (single char)
and ``??`` (double char) to match all stations.

Input Data
----------
``MHO_PolarizationRelabeler`` operates on ``multitone_pcal_type`` containers (a 3D axis pack: polarization x time x frequency). It modifies the polarization axis labels in place.

``MHO_PolarizationProductRelabeler`` operates on both ``visibility_type`` and ``weight_type`` containers (4D axis pack: polarization-product x channel x time x frequency). It modifies the polarization-product axis labels in place.

Algorithm
---------
Both classes inherit from ``MHO_UnaryOperator`` and have no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

**MHO_PolarizationRelabeler (``ExecuteInPlace``):**

1. Retrieve the polarization axis (``MTPCAL_POL_AXIS``) from the multitone pcal container.
2. Call ``IsApplicable`` to determine whether this pcal container belongs to a station that the user targeted. The method retrieves the Mark4 ID (``station_mk4id``) and 2-character station code (``station_code``) from the container's tags and compares them against the configured station identities.
3. If applicable, iterate over every polarization index on the axis. For each index, read the current label: if it matches ``pol1``, replace with ``pol2``; if it matches ``pol2``, replace with ``pol1``. Labels not matching either are left unchanged.

**MHO_PolarizationProductRelabeler (``ExecuteInPlace``):**

1. Retrieve the polarization-product axis (``POLPROD_AXIS``) from the visibility or weight container.
2. For each of the two stations in the baseline (index 0 = reference, index 1 = remote):

   a. Call ``IsApplicable`` using the appropriate station tags (``reference_station_mk4id``/``reference_station`` or ``remote_station_mk4id``/``remote_station``).
   b. If applicable, iterate over every polarization-product index. For each product label (e.g., ``XY``), swap the character at position ``st_idx`` if it matches ``pol1`` or ``pol2``.

Thus the polarization product ``XY`` on a baseline where only the reference station is targeted
and the swap pair is (X, Y) becomes ``YY`` (only the first character changes).

If both stations are targeted, a single operator instance processes both station positions sequentially against the same,
already-partially-mutated pol-product axis within one ``ExecuteInPlace`` call.
There is no separate operator per station. Starting from ``XY``, the
reference-station pass (``st_idx=0``) turns it into ``YY``, then the
remote-station pass (``st_idx=1``, now reading the already-mutated ``YY``) turns the
second character (``Y``, matching ``pol2``) into ``pol1`` = ``X``, giving ``YX``.
Targeting both stations therefore performs a genuine two-character swap, not a round-trip back to the original label.

Effect on Data
--------------
Both operators modify only metadata (axis labels). ``MHO_PolarizationRelabeler``
swaps single-character polarization labels on the pcal container's polarization
axis. ``MHO_PolarizationProductRelabeler`` swaps the corresponding character
within two-character polarization-product labels on the visibility and weight
containers. No visibility values, weights, or pcal phasors are altered. The
operator is designed so that both the visibility and the associated pcal data
carry consistent polarization labeling after correction.
