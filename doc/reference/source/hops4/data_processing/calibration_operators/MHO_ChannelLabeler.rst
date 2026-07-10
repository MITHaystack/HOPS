MHO\_ChannelLabeler
===================

Purpose
-------
``MHO_ChannelLabeler`` attaches fourfit-style channel labels
(e.g., ``a``, ``b``, ...) to each frequency channel in both visibility and
weight containers. It operates in two modes: a user-supplied label-to-frequency
mapping (triggered by the ``chan_ids`` control statement) or a default mode
(triggered internally by ``default_chan_ids``) that assigns labels
alphabetically in order of increasing sky frequency.

Control File Trigger
--------------------
- **Keyword:** ``chan_ids`` (user-supplied), category ``labeling``, priority 0.1
- **Keyword:** ``default_chan_ids`` (internal fallback), category ``default``, priority 0.1

Note: the ``default`` category is built even earlier in the pipeline than ``labeling``.

.. list-table:: Parameters for ``chan_ids``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - channel_names
     - string
     - Comma-separated channel label strings (e.g. ``a,b,c,d``).
   * - channel_frequencies
     - list_real
     - Sky frequencies (Hz) corresponding to each label.

The ``default_chan_ids`` variant takes no parameters and assigns labels in order of increasing frequency starting with ``a``.

Input Data
----------
This operator is a template class instantiated for both ``visibility_type`` and ``weight_type`` containers. It modifies the channel axis metadata in both containers.

Algorithm
---------
``MHO_ChannelLabeler`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

**Execution (``ExecuteInPlace``):**

1. Retrieve the channel axis from the container and determine the number of channels.
2. Check for double-sideband (DSB) interval labels on the channel axis. If DSB regions exist, channels within a DSB pair will receive a suffixed label (``-`` for one partner, ``+`` for the other) rather than a plain label.
3. **Default mode** (no user-supplied map):

   a. Generate labels for channel indices 0, ..., N_chan-1 using the ``EncodeValueToLabel`` utility, which maps integer indices to base-52 strings (0 -> ``a``, 1 -> ``b``, ..., 26 -> ``A``, ..., 52 -> ``Z``, 53 -> ``aa``, etc.).
   b. Iterate over all channels in frequency order. For each channel that does not already have a ``channel_label``:

      i. Assign the next label from the generated sequence.
      ii. If the channel has a DSB partner (indicated by the ``dsb_partner`` index label), append ``-`` to the channel's label and ``+`` to its partner's label, then assign both.

4. **User-supplied map mode**:

   a. For each (label, frequency) pair in the user map, perform a brute-force search over all channel frequencies. A channel is matched if ``|f_map - f_chan| < eps``, where ``eps = 10^-4`` (configurable via ``SetTolerance``).
   b. When a match is found, insert the label. If the channel has a DSB partner, apply the ``-``/``+`` suffix convention as above.


Effect on Data
--------------
The operator modifies only metadata on the channel axis of both visibility and
weight containers. It inserts a ``channel_label`` key-value pair on each channel
index that does not already have one. For channels that are members of a DSB
pair, the labels are suffixed with ``-`` or ``+`` to distinguish DSB partners.
No visibility or weight values are altered.
