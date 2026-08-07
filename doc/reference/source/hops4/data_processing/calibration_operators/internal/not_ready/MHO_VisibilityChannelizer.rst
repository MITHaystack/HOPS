MHO\_VisibilityChannelizer and MHO\_WeightChannelizer
======================================================

Purpose
-------

This operator converts unchannelized (3-D) visibility data into
channelized (4-D) form by grouping contiguous spectral-point ranges
into logically distinct channels. It is an internal utility that
prepares data for downstream operators expecting the standard 4-D
visibility layout. A companion class, ``MHO_WeightChannelizer``, performs
the equivalent transformation on weight data.

Control File Trigger
--------------------

This operator is internal and has no control file keyword. It is invoked automatically by the data pipeline when the input store contains unchannelized data that must be reshaped into the channelized format expected by calibration and fringe-fitting operators.

Input Data
----------

**MHO\_VisibilityChannelizer**

Takes a ``uch_visibility_store_type`` (unchannelized visibility store) as input. This is a 3-D container with axes:

- **UCH\_POLPROD\_AXIS** -- polarization product axis
- **UCH\_TIME\_AXIS** -- accumulation period axis
- **UCH\_FREQ\_AXIS** -- flat (unchannelized) frequency axis carrying interval labels that define channel boundaries

Each channel is described by an interval label on the frequency axis keyed by ``channel``, which contains JSON fields: ``sky_freq``, ``bandwidth``, ``net_sideband``, ``upper_index``, ``lower_index``, ``chan_id``, and ``frequency_band``.

**MHO\_WeightChannelizer**

Takes a ``uch_weight_store_type`` (unchannelized weight store) with the same 3-D axis structure. Unlike visibilities, weights have one scalar value per (channel, AP) pair, so the output frequency axis has size 1.

Algorithm
---------

Both channelizers follow the same two-phase approach:

**Phase 1 -- Initialization.**

1. Retrieve all interval labels on the frequency axis keyed by ``channel``. Each label defines a contiguous sub-range [lower\_index, upper\_index) of spectral points belonging to one logical channel.
2. Sort the channel labels by center frequency. The center frequency of a channel is computed as:

.. math::

   f_{\rm center} = f_{\rm sky} + s \cdot \frac{B}{2}

where :math:`f_{\rm sky}` is the sky frequency, :math:`B` is the bandwidth, and :math:`s = +1` for upper sideband (USB) or :math:`s = -1` for lower sideband (LSB).

3. Verify that all channels have the same number of spectral points. If not, a warning is issued and the smallest channel length is used (for visibilities) or a fixed length of 1 (for weights).
4. Resize the output container to [N\_polprod][N\_channel][N\_AP][N\_freq\_per\_channel], copying axis metadata (polarization labels, time labels, channel sky frequencies, and relative frequency offsets).

**Phase 2 -- Execution.**

1. For each sorted channel, retrieve its ``lower_index`` and ``upper_index`` from the frequency-axis interval label.
2. For each (polprod, AP, spectral point within the channel's range), copy the data into the output container's corresponding (polprod, channel, AP, freq-offset) location.
3. Attach a fresh channel label object to the output channel axis containing all channel metadata fields.
4. Set standard axis metadata (name and units) on all four output axes.

For ``MHO_WeightChannelizer``, the inner spectral-point loop is replaced by a single copy from the lower-index position (since weights are scalar per channel/AP). The output frequency axis is fixed to size 1.

Effect on Data
--------------

This operator produces a new ``visibility_store_type`` (or ``weight_store_type``) container.
The input is not modified. The output has a 4-D axis structure where
the CHANNEL\_AXIS replaces the flat frequency axis, and a new FREQ\_AXIS
represents intra-channel spectral bins. The output frequency axis uses
relative offsets (each bin is :math:`f - f_0` where :math:`f_0` is the
first spectral-point frequency), while the channel axis carries the sky
frequency for each channel. All channel metadata (sideband, bandwidth,
frequency band, channel ID) is preserved as interval labels on the output channel axis.
