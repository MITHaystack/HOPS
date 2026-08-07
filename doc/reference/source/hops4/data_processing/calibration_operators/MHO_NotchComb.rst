MHO\_NotchComb
==============

Purpose
-------
``MHO_NotchComb`` applies a periodic ``comb`` of notches across the frequency
spectrum of visibility data. Notches are defined by a starting offset, a
repetition period, and a notch width, producing regularly spaced frequency
regions that are zeroed out. This operator is useful for excising known
periodic interferers (e.g., p-cal tones from a neighboring station) that recur
at regular frequency intervals.

Control File Trigger
--------------------
- **Keyword:** ``notch_comb``
- **Category:** flagging
- **Priority:** 4.5

.. list-table:: Parameters for ``notch_comb``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - offset
     - real
     - Frequency offset (in MHz) at which the first notch center is placed. Defaults to 0.
   * - period
     - real
     - Repetition period (in MHz) between consecutive notch centers. Must be positive.
   * - width
     - real
     - Width (in MHz) of each individual notch. Must be positive and less than the period.

The builder (``MHO_NotchCombBuilder``) silently takes the absolute value of ``offset``, ``period``, and ``width`` (so negative inputs are accepted but treated as their positive equivalent), and separately rejects (with an error) any configuration where ``width`` exceeds ``period``. The builder retrieves both the visibility and weight containers from the container store and configures the operator with all three parameters.

Input Data
----------
This operator acts on the ``visibility_type`` container and simultaneously
modifies the ``weight_type`` container (supplied via ``SetWeights``).

Algorithm
---------
``MHO_NotchComb`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

**Execution (``ExecuteInPlace``):**

1. If either ``period`` or ``width`` is non-positive, the operator is a no-op and returns immediately.
2. Retrieve the channel axis and frequency axis from the visibility container.
3. Iterate over every frequency channel:

   a. Retrieve the channel's sky frequency, ``bandwidth`` label, and ``net_sideband`` label (``U`` for upper sideband, ``L`` for lower sideband).
   b. Compute the channel's lower and upper frequency limits using ``DetermineChannelFrequencyLimits``:

      .. math::

         [f_{\rm low}, f_{\rm high}] = \begin{cases}
         [f_{\rm sky},\ f_{\rm sky} + B] & \text{USB} \\
         [f_{\rm sky} - B,\ f_{\rm sky}] & \text{LSB}
         \end{cases}

      where :math:`f_{\rm sky}` is the sky frequency and :math:`B` is the bandwidth.
   c. Determine which notch centers fall within (or overlap) the channel's frequency span. A notch center is defined as:

      .. math::

         f_{\rm notch}(n) = f_{\rm offset} + n \cdot P

      where :math:`f_{\rm offset}` is the notch offset, :math:`P` is the period, and :math:`n` is an integer. The range of :math:`n` values is computed as:

      .. math::

         \begin{aligned}
         n_{\rm lower} &= \left\lfloor \frac{f_{\rm low} - f_{\rm offset}}{P} \right\rfloor \\
         n_{\rm upper} &= \left\lceil \frac{f_{\rm high} - f_{\rm offset}}{P} \right\rceil
         \end{aligned}

      The floor/ceil are chosen so that notches outside the nominal channel range but overlapping due to finite notch width are still captured.
   d. For each integer :math:`m = 0, \dots, n_{\rm upper} - n_{\rm lower} - 1`:

      i. Compute the notch center: :math:`f_{\rm notch} = f_{\rm offset} + (n_{\rm lower} + m) \cdot P`.
      ii. Compute the notch boundaries: :math:`[f_{\rm notch} - W/2,\ f_{\rm notch} + W/2]` where :math:`W` is the notch width.
      iii. Use ``FindIntersection`` to check whether the notch interval overlaps the channel interval.
      iv. If there is overlap, iterate over all spectral points within the channel. For each spectral point, compute the absolute frequency:

          .. math::

             f_{\rm sp} = f_{\rm sky} + s_b \cdot \Delta f_{\rm sp}

          where :math:`s_b = +1` for USB and :math:`s_b = -1` for LSB, and :math:`\Delta f_{\rm sp}` is the frequency-axis value for that spectral point.
      v. If :math:`f_{\rm sp}` falls within the notch boundaries, zero out the visibility slice for that spectral point across all polarization products and accumulation periods, and increment a counter.
   e. After all notches for the channel are processed, compute the used bandwidth fraction:

      .. math::

         \text{frac} = \frac{N_{\rm freq} - N_{\rm zeroed}}{N_{\rm freq}}

      and the rescaling factor:

      .. math::

         \text{factor} = \begin{cases} 1/\text{frac} & \text{frac} > 0 \\ 0 & \text{otherwise} \end{cases}
   f. Multiply the weight slice for the channel by the rescaling factor.
   g. Store the metadata keys ``used_bandwidth_fraction`` and ``rescaling_factor`` on the channel axis.

Effect on Data
--------------
For each channel, spectral points whose absolute frequency falls within any
notch interval are zeroed across all polarization products and accumulation
periods. The weight container is rescaled per channel: each channel's weights
are multiplied by 1/frac where frac is the fraction of spectral points retained.
Two metadata keys are recorded on the channel axis for every
channel: ``used_bandwidth_fraction`` (the fraction of spectral points not
zeroed) and ``rescaling_factor`` (the inverse of that fraction). This weight
rescaling compensates approximately for the reduced effective bandwidth.
