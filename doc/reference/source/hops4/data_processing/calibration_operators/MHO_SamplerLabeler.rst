MHO\_SamplerLabeler
===================

Purpose
-------
``MHO_SamplerLabeler`` attaches per-channel sampler index labels to visibility
data so that downstream operators (specifically phase-calibration processing) can
look up the station sampler delay and ambiguity information for each channel.
It maps channel labels (e.g., ``a``, ``b``, ...) to integer sampler IDs for
both the reference and remote stations. This information is used by
the ``MHO_MultitonePhaseCorrection`` operator during ambiguity resolution.

Control File Trigger
--------------------
- **Internal operator name:** ``sampler_labeler`` (not a keyword the user writes directly;
  it has no control-file format entry and is auto-built whenever a ``samplers`` statement is
  present)
- **Category:** labeling
- **Priority:** 0.9
- **Parameters:** None (sampler-to-channel mapping is read from the control file's ``samplers`` statement, either globally or per-station).

The sampler mapping is specified in the control file using the ``samplers``
keyword under the ``station`` section. For example:

.. code-block:: none

    samplers 4 abcdefgh ijklmnop qrstuvwx yzABCDEF

This declares 4 samplers, each handling the channels listed after the count.
The mapping can be provided globally (under ``/control/station/samplers``)
or per-station (under ``/control/station/STATION_CODE/samplers``).

Input Data
----------
This operator is a template class instantiated for ``visibility_type`` containers (4D axis pack: polarization-product x channel x time x frequency). It reads the existing ``channel_label`` metadata from each channel index and inserts two new key-value pairs: ``ref_sampler_index`` and ``rem_sampler_index``.

Algorithm
---------
``MHO_SamplerLabeler`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

**Execution (``ExecuteInPlace``):**

1. **Construct channel-to-sampler maps:** For both the reference and remote station, call ``ConstructChannelToSamplerIDMap``, which iterates over the sampler channel-set strings. Each string (e.g., ``abcdefgh``) is split into individual channel labels using ``SplitChannelLabels``. The resulting map assigns each channel label to the integer index of its sampler (0-based).
2. **Channel label parsing:** The ``SplitChannelLabels`` method handles two formats:

   a. Comma-delimited strings (e.g., ``a,b,c,d``) are tokenized using ``MHO_Tokenizer``.
   b. Compact strings (e.g., ``abcdefgh``) are split character by character.

3. **Attach sampler indices:** Iterate over all channels in the visibility container. For each channel:

   a. Retrieve the channel's ``channel_label`` (e.g., ``a``).
   b. Look up the label in the reference station's channel-to-sampler map. If found, insert the ``ref_sampler_index`` key-value pair on that channel index.
   c. Look up the label in the remote station's channel-to-sampler map. If found, insert the ``rem_sampler_index`` key-value pair.

Effect on Data
--------------
The operator modifies only metadata on the channel axis of the visibility
container. For each channel that has a ``channel_label`` matching an entry in
the sampler mapping, it inserts ``ref_sampler_index`` and/or
``rem_sampler_index`` key-value pairs. No visibility values are altered.
Channels without a matching label are left unchanged
(no sampler index is inserted). The sampler indices are later used by pcal
operators to look up per-sampler delay corrections.
