Data Labeling and Organization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The data labeling and organization components provide utilities for organizing and
labeling data channels, samplers, and channel quantities for fringe fitting operations.

:hops:`MHO_ChannelLabelerBuilder`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ChannelLabelerBuilder`
Primary Functionality                           Builds a channel labeler operator
Key Features                                    | Inherits from MHO_OperatorBuilder
                                                | Provides Build() method for channel labeler initialization
                                                | Organizes channels for fringe fitting operations
=============================================== ====================================================================

The :hops:`MHO_ChannelLabelerBuilder` class builds a channel labeler operator that
organizes and labels data channels for fringe fitting operations.

:hops:`MHO_SamplerLabelerBuilder`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_SamplerLabelerBuilder`
Primary Functionality                           Builds a MHO_SamplerLabeler operator
Key Features                                    | Initializes and builds MHO_SamplerLabeler operator
                                                | Groups channels by sampler for organization
                                                | Provides sampler-based channel grouping
=============================================== ====================================================================

The :hops:`MHO_SamplerLabelerBuilder` class builds a sampler labeler operator that
groups channels by their associated samplers. This organization is necessary for
proper data processing in VLBI systems where multiple channels may be associated
with the same sampler hardware (and thus share physical hardware-related delays).

:hops:`MHO_ChannelUtilities.hh`
-------------------------------

=============================================== ====================================================================
Header                                          :hops:`MHO_ChannelUtilities.hh`
Primary Functionality                           Free functions for mapping per-channel control quantities
Key Features                                    | Maps channel names to corresponding values
                                                | Handles comma-separated or concatenated channel names
                                                | Logs an error and truncates to the shorter input if channel names and values counts don't match
                                                | MapChannelQuantities() function for channel-to-value mapping
=============================================== ====================================================================

The :hops:`MHO_ChannelUtilities.hh` header provides a utility for storing and mapping
per-channel control quantities, which is a typical task for fourfit control files.
It handles the mapping between channel names and their corresponding values.

The functions can process channel names in various formats (comma-separated or
concatenated 1-character names). If the number of channel names and values don't
match, an error is logged and the resulting map is truncated to the shorter of the
two inputs.
