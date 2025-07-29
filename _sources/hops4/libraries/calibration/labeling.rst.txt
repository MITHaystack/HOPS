Labeling Operators
==================

MHO_ChannelLabeler
------------------

======================= ======================================================
Class                   :hops:`MHO_ChannelLabeler`
Operator Type           Unary
Operator Category       labeling
Argument Data Type      :hops:`visibility_type` or :hops:`weight_type`
Priority Value          0.1
Control File Keyword    ``chan_ids``
======================= ======================================================

Applies 'fourfit' labels to each channel (e.g. 'a', 'b',...),  if no user-defined 
frequency to label map is supplied then the default mapping is in the order of 
frequency low to high, starting with the character 'a'. If the number of channels 
exceeds 64, multi-character labels will be used. Attaches index labels to each 
element of the channel axis of the input data object. 
The key name is 'channel_label'.


MHO_SamplerLabeler
------------------

======================= ======================================================
Class                   :hops:`MHO_SamplerLabeler`
Operator Type           Unary
Operator Category       labeling
Argument Data Type      :hops:`visibility_type` or :hops:`weight_type`
Priority Value          0.9
Control File Keyword    ``samplers``, (see also ``sampler_delay``)
======================= ======================================================

When the 'samplers' keyword is encountered, this operator loops over all channels
and inserts a label for each channel which contains the specified sampler index.
This is a look-up mechanism for associating the polarization-specific sampler_delay 
value with the appropriate set of channels (i.e. those where are collected by the 
same digital backend). This quantity is used by the MHO_MultitonePhaseCorrection
operator.