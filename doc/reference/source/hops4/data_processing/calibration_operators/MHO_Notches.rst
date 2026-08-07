MHO\_Notches
============

Purpose
-------
``MHO_Notches`` zeroes out visibility data at arbitrary, user-specified
frequency ranges within each channel. Unlike ``MHO_NotchComb``, which produces
periodic notches, this operator accepts an explicit list of (lower, upper)
frequency boundaries, allowing precise targeting of known interferers or RFI
bands at arbitrary frequencies.

Control File Trigger
--------------------
- **Keyword:** ``notches``
- **Category:** flagging
- **Priority:** 4.5

.. list-table:: Parameters for ``notches``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - value
     - list_real
     - List of frequency values (in MHz) specifying notch boundaries. Values must come in pairs (lower, upper) representing the edges of each notch region. The total number of values must be an even number.

The builder (``MHO_NotchesBuilder``) retrieves the list of real values from the
control file, validates that the count is even, and passes them to the
operator's ``SetNotchBoundaries`` method. The header method pairs consecutive
values into (low, high) intervals, warning if an odd number of values is
supplied (dropping the last).

Input Data
----------
This operator acts on the ``visibility_type`` container and simultaneously modifies the ``weight_type`` container (supplied via ``SetWeights``).

Algorithm
---------
``MHO_Notches`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

**Execution (``ExecuteInPlace``):**

1. Retrieve the channel axis and frequency axis from the visibility container.
2. Iterate over every frequency channel:

   a. Initialize a zeroed counter for tracking excised spectral points.
   b. For each notch interval (f_notch_low, f_notch_high):

      i. Retrieve the channel's sky frequency, ``bandwidth`` label, and ``net_sideband`` label (``U`` for upper sideband, ``L`` for lower sideband).
      ii. Compute the channel's lower and upper frequency limits using ``DetermineChannelFrequencyLimits``:

          .. math::

             [f_{\rm low}, f_{\rm high}] = \begin{cases}
             [f_{\rm sky},\ f_{\rm sky} + B] & \text{USB} \\
             [f_{\rm sky} - B,\ f_{\rm sky}] & \text{LSB}
             \end{cases}

          where :math:`f_{\rm sky}` is the sky frequency and :math:`B` is the bandwidth.
      iii. Use ``FindIntersection`` to check whether the notch interval overlaps the channel interval.
      iv. If there is overlap, iterate over all spectral points within the channel. For each spectral point, compute the absolute frequency:

          .. math::

             f_{\rm sp} = f_{\rm sky} + s_b \cdot \Delta f_{\rm sp}

          where :math:`s_b = +1` for USB and :math:`s_b = -1` for LSB.
      v. If :math:`f_{\rm sp}` falls within the notch boundaries (:math:`f_{\rm notch\_low} < f_{\rm sp} < f_{\rm notch\_high}`), zero out the visibility slice for that spectral point across all polarization products and accumulation periods, and increment the counter.
   c. After all notches are processed, compute the used bandwidth fraction:

      .. math::

         \text{frac} = \frac{N_{\rm freq} - N_{\rm zeroed}}{N_{\rm freq}}

      and the rescaling factor:

      .. math::

         \text{factor} = \begin{cases} 1/\text{frac} & \text{frac} > 0 \\ 0 & \text{otherwise} \end{cases}
   d. Multiply the weight slice for the channel by the rescaling factor.
   e. Store the metadata keys ``used_bandwidth_fraction`` and ``rescaling_factor`` on the channel axis.

Effect on Data
--------------
For each channel, spectral points whose absolute frequency falls within
any user-specified notch interval are zeroed across all polarization products
and accumulation periods. The weight container is rescaled per channel: each
channel's weights are multiplied by 1/frac where frac is the fraction of
spectral points retained. Two metadata keys are recorded on the channel
axis: ``used_bandwidth_fraction`` (the fraction of spectral points not zeroed)
and ``rescaling_factor`` (the inverse of that fraction).
