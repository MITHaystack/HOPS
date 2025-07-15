Data Operator: `MHO_ChannelLabeler`
===================================

Applies 'fourfit' labels to each channel (e.g. 'a', 'b',...),  if no user-defined 
frequency to label map is supplied then the default mapping is in the order of 
frequency low to high, starting with the character 'a'. If the number of channels exceeds 64, 
multi-character labels will be used. Attaches index labels to each element of the 
channel axis of the input data object. The key name is 'channel_label'.

General Information
-------------------
- **Class**: :hops:`MHO_ChannelLabeler`
- **Operator Type**: Unary
- **Operator Category**: labeling
- **Argument Data Type**: :hops:`visibility_type` or :hops:`weight_type`
- **Priority Value**: 0.1
- **Control File Keyword**: ``chan_ids``
