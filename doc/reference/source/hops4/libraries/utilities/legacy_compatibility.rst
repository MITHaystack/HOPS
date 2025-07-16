Legacy Compatibility Tools
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The utilities library includes some tools for handling legacy HOPS3 files/formats.

:hops:`MHO_LegacyRootCodeGenerator`
-----------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_LegacyRootCodeGenerator`
Category                                        Legacy Compatibility
Template Parameters                             None
Configuration Parameters                        None
Primary Functionality                           Generates 6-character timestamp-based root codes
Key Features                                    | HOPS3-compatible root code generation
                                                | Time-based sequential code assignment
                                                | Collision avoidance for sub-delta-T intervals (batch processing)
                                                | Batch code generation for experiments
=============================================== ====================================================================

The :hops:`MHO_LegacyRootCodeGenerator` class generates 6-character 
timestamp-based root codes compatible with legacy HOPS3 file naming conventions.

:hops:`MHO_StationCodeMap`
--------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_StationCodeMap`
Category                                        Legacy Compatibility
Template Parameters                             None
Configuration Parameters                        Optional legacy code usage
Primary Functionality                           Maps between 1-character and 2-character station codes
Key Features                                    | File-based station code initialization
                                                | Legacy DiFX2Mark4 code compatibility
                                                | Automatic code assignment for new stations
                                                | Bidirectional code mapping
=============================================== ====================================================================

The :hops:`MHO_StationCodeMap` class handles mapping between 1-character 
and 2-character station representations with support for legacy code assignments.

:hops:`legacy_hops_date`
------------------------

=============================================== ====================================================================
Class                                           :hops:`legacy_hops_date`
Category                                        Legacy Compatibility
Template Parameters                             None
Configuration Parameters                        None
Primary Functionality                           Legacy HOPS3 date structure compatibility
Key Features                                    | Compatible with legacy HOPS3 date struct
                                                | Prevents namespace collisions with modern *date* library
                                                | Maintains original field layout and types
=============================================== ====================================================================

The :hops:`legacy_hops_date` struct provides compatibility with the legacy HOPS3 date 
struct.
