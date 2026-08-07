MHO\_WeightChannelizer
=======================

Purpose
-------

This operator converts unchannelized (3-D) weight data into channelized
(4-D) form. It is the companion to ``MHO_VisibilityChannelizer``, performing
the same channel-discovery and reorganization logic on the weight store. The
key difference is that weights are scalar values per (channel, accumulation
period) pair, so the output frequency axis has size 1 rather than
the per-channel spectral-point count.

Control File Trigger
--------------------

This operator is internal and has no control file keyword.
It is invoked automatically by the data pipeline alongside ``MHO_VisibilityChannelizer``
to ensure weights remain dimensionally aligned with channelized visibilities.

Input Data
----------

The operator takes a ``uch_weight_store_type`` (unchannelized weight store) as input. This is a 3-D container with axes:

- **UCH\_POLPROD\_AXIS** -- polarization product axis
- **UCH\_TIME\_AXIS** -- accumulation period axis
- **UCH\_FREQ\_AXIS** -- flat (unchannelized) frequency axis carrying interval labels keyed by ``channel``

Each channel interval label contains the same metadata as in the visibility store: ``sky_freq``, ``bandwidth``, ``net_sideband``, ``upper_index``, ``lower_index``, ``chan_id``, and ``frequency_band``.

Algorithm
---------

The algorithm mirrors ``MHO_VisibilityChannelizer`` with one important distinction:

**Phase 1 -- Initialization.**

1. Collect all channel interval labels from the frequency axis and sort by center frequency:

.. math::

   f_{\rm center} = f_{\rm sky} + s \cdot \frac{B}{2}

where :math:`s = +1` for USB, :math:`s = -1` for LSB.

2. Verify uniform channel sizes (warning if non-uniform).
3. Set the output frequency-axis size to 1 (fixed, regardless of per-channel spectral-point count). The output is resized to [N\_polprod][N\_channel][N\_AP][1].
4. Copy axis metadata from input to output.

**Phase 2 -- Execution.**

1. For each channel, retrieve the ``lower_index`` from the frequency-axis interval label.
2. For each (polprod, AP), copy a single weight value from the input's lower-index position:

.. math::

   \mathtt{out}(pp, ch, t, 0) \leftarrow \mathtt{in}(pp, t, \mathtt{lower\_index})

3. Attach a fresh channel label to the output channel axis.
4. Set axis names and units.

Effect on Data
--------------

This operator produces a new ``weight_store_type`` container with a 4-D axis
structure. The input is not modified. The output frequency axis has size 1
(since a single weight applies to all spectral bins within a channel/AP).
Channel axis labels and metadata are preserved. The resulting weight container
is dimensionally compatible with the output of ``MHO_VisibilityChannelizer``,
allowing element-wise weighting of the channelized visibilities.
