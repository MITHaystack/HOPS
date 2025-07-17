Data Labeling and Organization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The data labeling and organization components provide utilities for organizing and 
labeling data channels, samplers, and channel quantities for fringe fitting operations.

:hops:`MHO_ChannelLabelerBuilder`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ChannelLabelerBuilder`
Primary Functionality                           Builds a channel labeler operator
Key Features                                    | Inherits from MHO_OperatorBuilder and MHO_ChannelQuantity
                                                | Provides Build() method for channel labeler initialization
                                                | Returns bool indicating success of initialization
                                                | Organizes channels for fringe fitting operations
=============================================== ====================================================================

The :hops:`MHO_ChannelLabelerBuilder` class builds a channel labeler operator that 
organizes and labels data channels for fringe fitting operations. It combines the 
functionality of operator building with channel quantity management.

The builder provides a Build() method that initializes the channel labeler operator 
and returns a boolean indicating the success of the initialization process.

:hops:`MHO_SamplerLabelerBuilder`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_SamplerLabelerBuilder`
Primary Functionality                           Builds a MHO_SamplerLabeler operator
Key Features                                    | Initializes and builds MHO_SamplerLabeler operator
                                                | Groups channels by sampler for organization
                                                | Returns bool indicating success of initialization
                                                | Provides sampler-based channel grouping
=============================================== ====================================================================

The :hops:`MHO_SamplerLabelerBuilder` class builds a sampler labeler operator that 
groups channels by their associated samplers. This organization is essential for 
proper data processing in VLBI systems where multiple channels may be associated 
with the same sampler hardware.

The builder initializes the MHO_SamplerLabeler operator and provides feedback on 
the success of the initialization process.

:hops:`MHO_ChannelQuantity`
---------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ChannelQuantity`
Primary Functionality                           Stores and maps per-channel control quantities
Key Features                                    | Maps channel names to corresponding values
                                                | Handles comma-separated or concatenated channel names
                                                | Returns empty map if channel names and values don't match
                                                | MapChannelQuantities() method for channel-to-value mapping
=============================================== ====================================================================

The :hops:`MHO_ChannelQuantity` class provides a utility for storing and mapping 
per-channel control quantities, which is a typical task for fourfit control files. 
It handles the mapping between channel names and their corresponding values.

The class can process channel names in various formats (comma-separated or 
concatenated 1-character names) and provides robust error handling by returning 
an empty map when channel names and values counts don't match.
