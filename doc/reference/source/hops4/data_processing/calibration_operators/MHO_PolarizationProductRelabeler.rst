MHO_PolarizationProductRelabeler
=================================

Purpose
-------
Swaps polarization-product labels
(e.g., XX $\leftrightarrow$ YY, RL $\leftrightarrow$ LR) on visibility and
weight data containers. This operator modifies the 2-character
polarization-product strings on the polprod axis, swapping the character at the
position corresponding to a specific station (reference or remote). It is the
companion to ``MHO_PolarizationRelabeler``, which operates on the
single-character polarization labels of ``multitone_pcal_type``
containers.

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
   * - ``pol1``
     - string
     - First polarization label to swap (single character, e.g., ``X``)
   * - ``pol2``
     - string
     - Second polarization label to swap (single character, e.g., ``Y``)

The builder instantiates this operator for both ``visibility_type``
and ``weight_type`` containers. Station applicability is determined by the
control file's station targeting (ref, rem, or both).

Input Data
----------
This is a template unary operator ``MHO_UnaryOperator<XArrayType>`` instantiated for:

- ``visibility_type`` -- primary visibility data
- ``weight_type`` -- associated weights (labels must stay in sync)

Algorithm
---------
The operator iterates over both stations (reference at index 0, remote at index 1). For each applicable station, it scans the polarization-product axis and swaps the character at the station's position in the 2-character product string.

**Station matching**

The operator checks if the container's station metadata matches any of the configured station identifiers. A 1-character identifier is matched against the Mark4 ID; a 2-character identifier is matched against the 2-letter station code. Wildcards (``?`` or ``??``) match any station.

**Label swapping**

For each polarization product string ``pprod``:

- If ``pprod[st_idx]`` equals ``pol1``, replace with ``pol2``
- If ``pprod[st_idx]`` equals ``pol2``, replace with ``pol1``

Only the character at position ``st_idx`` is modified; the other character is left unchanged.

Effect on Data
--------------
This operator modifies the polarization-product labels on the polprod axis of
visibility and weight containers. For example, if ``pol1=X``, ``pol2=Y``, and
the reference station (index 0) is targeted, then ``XX``
becomes ``YX``, ``XY`` becomes ``YY``, ``YX`` remains ``YX``, and ``YY``
becomes ``XY``. The data values themselves are unchanged; only the metadata
labels are modified.
