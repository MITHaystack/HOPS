MHO\_DCBlock
============

Purpose
-------
``MHO_DCBlock`` removes the DC spectral point from each channel's visibility
data by zeroing the visibility at the spectral point nearest to the local
oscillator frequency. The DC point corresponds to the spectral channel where
the sky frequency equals the LO frequency, which typically contains spurious
power or corrupted data. Which spectral point is the DC point depends on the
channel's sideband: for lower sideband (LSB) channels, the DC point is the
highest-frequency spectral bin; for upper sideband (USB) channels, it is the
lowest-frequency spectral bin (index 0).

Control File Trigger
--------------------
- **Keyword:** ``dc_block``
- **Category:** flagging
- **Priority:** 3.5

.. list-table:: Parameters for ``dc_block``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - dc_block
     - boolean
     - When ``true``, enables DC blocking; when ``false``, the operator is not created.

Input Data
----------
This operator acts on the ``visibility_type`` container.

Algorithm
---------
``MHO_DCBlock`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

**Execution (``ExecuteInPlace``):**

1. Retrieve the channel axis and frequency axis from the visibility container.
2. Iterate over every frequency channel:

   a. Retrieve the channel's ``net_sideband`` label (``U`` for upper sideband, ``L`` for lower sideband).
   b. Determine the DC spectral index:

      - USB (``net_sideband`` = ``U``): DC index is 0 (lowest spectral bin).
      - LSB (``net_sideband`` = ``L``): DC index is ``N_freq - 1`` (highest spectral bin).

   c. Zero out all visibility values at spectral index ``dc_index`` for that channel across all polarization products and all accumulation periods. This is accomplished by multiplying a slice view covering all pol-products and all APs at (channel, dc_index) by zero.

Effect on Data
--------------
For each channel, exactly one spectral point (the DC point) is zeroed across
all polarization products and all accumulation periods. The DC spectral index
depends on the channel's sideband: index 0 for USB, index ``N_freq - 1`` for LSB.
The operator does not modify data weights or bandwidth fractions, as the effect
on signal-to-noise ratio from removing a single spectral point is typically
negligible.
