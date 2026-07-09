MHO\_Passband
=============

Purpose
-------
``MHO_Passband`` selects a contiguous frequency range for either inclusion or
exclusion across all channels. Depending on the ordering of the two frequency
limits, the operator either cuts out a specific band of spectrum
(exclusion mode) or retains only a specific band and cuts everything
outside (inclusion mode). This provides a simple mechanism for restricting
analysis to a known clean frequency range or for removing a specific
interferer band.

Control File Trigger
--------------------
- **Keyword:** ``passband``
- **Category:** flagging
- **Priority:** 4.5

.. list-table:: Parameters for ``passband``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - value
     - list_real
     - List of exactly two frequency values (in MHz) specifying the band limits. The interpretation depends on their order: if ``first < second``, the band [first, second] is the inclusion range (everything outside is cut). If ``second < first``, the band [second, first] is the exclusion range (only this band is cut).

The builder (``MHO_PassbandBuilder``) validates that exactly two frequency
values are provided and passes them to the operator's ``SetPassband`` method.
The default behavior (when neither ordering is explicitly set) is exclusion mode,
which matches the legacy fourfit convention.

Input Data
----------
This operator acts on the ``visibility_type`` container and simultaneously
modifies the ``weight_type`` container (supplied via ``SetWeights``).

Algorithm
---------
``MHO_Passband`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

The operator supports two modes of operation, distinguished by the ``fIsExclusion`` flag set during ``SetPassband``:

**Exclusion mode** (:math:`f_{\rm second} < f_{\rm first}`):

The frequency band :math:`[f_{\rm low}, f_{\rm high}]` is excised; everything outside is retained.

1. Iterate over every frequency channel.
2. Retrieve the channel's sky frequency, ``bandwidth`` label, and ``net_sideband`` label.
3. Compute the channel's lower and upper frequency limits using ``DetermineChannelFrequencyLimits``:

   .. math::

      [f_{\rm chan\_low}, f_{\rm chan\_high}] = \begin{cases}
      [f_{\rm sky},\ f_{\rm sky} + B] & \text{USB} \\
      [f_{\rm sky} - B,\ f_{\rm sky}] & \text{LSB}
      \end{cases}

4. Use ``FindIntersection`` to check whether the exclusion band overlaps the channel interval.
5. If there is overlap, iterate over all spectral points:

   a. Compute the absolute frequency :math:`f_{\rm sp} = f_{\rm sky} + s_b \cdot \Delta f_{\rm sp}` where :math:`s_b = +1` for USB and :math:`s_b = -1` for LSB.
   b. If :math:`f_{\rm low} < f_{\rm sp} < f_{\rm high}`, zero out the visibility slice and increment the zeroed counter.
6. Compute the used bandwidth fraction and rescaling factor, apply to weights, and store metadata.

**Inclusion mode** (:math:`f_{\rm first} < f_{\rm second}`):

Only the frequency band :math:`[f_{\rm low}, f_{\rm high}]` is retained; everything outside is cut.

1. Iterate over every frequency channel.
2. Retrieve channel frequency information and compute the channel interval :math:`[f_{\rm chan\_low}, f_{\rm chan\_high}]`.
3. Use ``FindIntersection`` to check whether the inclusion band overlaps the channel interval.
4. If there **is** overlap:

   a. Iterate over all spectral points.
   b. For each spectral point, if :math:`f_{\rm sp} < f_{\rm low}` or :math:`f_{\rm sp} > f_{\rm high}` (outside the inclusion band), zero out the visibility slice and increment the counter.
   c. Compute the used bandwidth fraction and rescaling factor, apply to weights, and store ``used_bandwidth_fraction`` on the visibility channel axis and ``rescaling_factor`` on the weight container's channel axis.
5. If there is **no** overlap (channel is entirely outside the inclusion band):

   a. Zero out the entire channel (all spectral points, all polarization products, all accumulation periods).
   b. Multiply all weight entries for the channel by zero.
   c. Store ``used_bandwidth_fraction`` = 0.0 and ``rescaling_factor`` = 0.0.

Effect on Data
--------------
In exclusion mode, spectral points whose absolute frequency falls within the
specified band [f_low, f_high] are zeroed. In inclusion mode, all spectral
points outside [f_low, f_high] are zeroed, and channels with no overlap with
the inclusion band are entirely zeroed (both visibility and weight data).
In both modes, the weight container is rescaled per channel
using 1/frac where frac is the fraction of retained spectral points.
``used_bandwidth_fraction`` is stored on the visibility channel axis, and
``rescaling_factor`` is stored on the weight container's channel axis.
