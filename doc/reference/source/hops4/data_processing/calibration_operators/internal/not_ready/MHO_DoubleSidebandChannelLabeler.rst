MHO\_DoubleSidebandChannelLabeler
=================================

Purpose
-------
``MHO_DoubleSidebandChannelLabeler`` detects adjacent lower-sideband (LSB)
and upper-sideband (USB) channel pairs that share the same sky frequency and
bandwidth. Identified pairs are marked as double-sideband (DSB) channels by
inserting metadata labels on the channel axis, enabling downstream operators
to apply the legacy DSB treatment during fringe fitting.

Control File Trigger
--------------------
This operator is internal and not directly triggerable from the control file. It is used during the channel-labeling phase of the fringe-fitting pipeline to identify DSB channel pairs before channel labels are assigned.

Input Data
----------
This operator is a template class instantiated for any array type with a channel axis (typically ``visibility_type`` or ``weight_type``). It modifies only metadata on the channel axis.

Algorithm
---------
``MHO_DoubleSidebandChannelLabeler`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

**Execution (``ExecuteInPlace``):**

1. Retrieve the channel axis and determine the number of channels ``N_chan``.
2. Iterate over adjacent channel pairs ``(ch, ch+1)`` for ``ch = 0 .. N_chan - 2``:

   a. Retrieve the sky frequencies ``f1`` and ``f2`` for channels ``ch`` and ``ch+1``.
   b. Retrieve the bandwidth labels (``bandwidth``) for both channels (if present), stored as ``bw1`` and ``bw2``.
   c. Check if the channels share the same sky frequency and bandwidth: ``|f1 - f2| < eps`` and ``|bw1 - bw2| < eps``, where ``eps`` is the configured tolerance (default ``1e-6`` MHz, settable via ``SetTolerance``).
   d. If the frequency and bandwidth match, retrieve the ``net_sideband`` labels for both channels. If channel ``ch`` has ``net_sideband = "L"`` and channel ``ch+1`` has ``net_sideband = "U"``, this is a valid DSB pair:

      - Insert a ``double_sideband`` interval label spanning ``[ch, ch+1]`` with value ``true``.
      - Insert a ``dsb_partner`` index label on channel ``ch`` with value ``+1`` (relative offset to partner).
      - Insert a ``dsb_partner`` index label on channel ``ch+1`` with value ``-1`` (relative offset to partner).
      - Increment the DSB pair counter.

Effect on Data
--------------
The operator modifies only metadata on the channel axis. For each
identified LSB/USB pair, it inserts a ``double_sideband`` interval label
and per-channel ``dsb_partner`` index labels encoding the relative offset
to the partner channel. No visibility or weight values are altered.
