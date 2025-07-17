Information Extraction and Processing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The information extraction components provide functionality for extracting and processing 
data from various file formats used in VLBI processing, including .cor, .frng, root files, 
and VEX files.

:hops:`MHO_AFileDefinitions`
----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_AFileDefinitions`
Primary Functionality                           ASCII file format (.alist) definitions and keyword management
Key Features                                    | Static methods for format directory retrieval
                                                | Keyword name extraction for different file types
                                                | JSON-based afile format retrieval
                                                | Support for .cor, .frng, and root file formats
=============================================== ====================================================================

The :hops:`MHO_AFileDefinitions` class provides static methods for managing ASCII file 
format definitions and keyword extraction. It handles format directory retrieval, 
keyword name extraction, and JSON-based format data management for various VLBI file types 
(.cor, .frnge, and .root). Key functions include `GetFormatDirectory()` for format directory access and 
`GetAFileFormat()` for combined format data retrieval.

:hops:`MHO_AFileInfoExtractor`
------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_AFileInfoExtractor`
Primary Functionality                           Information extraction from .cor, .frng, and root files
Key Features                                    | Fringe file summarization
                                                | ALIST format conversion (versions 5 and 6)
                                                | Parameter type determination and conversion
                                                | JSON-based format management
=============================================== ====================================================================

The :hops:`MHO_AFileInfoExtractor` class extracts useful information from VLBI data 
files for ASCII (.alist) file generation. It provides fringe file summarization, .alist format 
conversion, and parameter type determination.
Key template functions include `ConvertToString()` for value formatting and 
`ConvertToAlistRow()` for .alist format conversion.

:hops:`MHO_VexInfoExtractor`
----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_VexInfoExtractor`
Primary Functionality                           VEX file information extraction and processing
Key Features                                    | Clock model extraction
                                                | Sample rate and sampler bits extraction
                                                | Coordinate system conversions
                                                | Time calculations and conversions
=============================================== ====================================================================

The :hops:`MHO_VexInfoExtractor` class extracts useful meta-data from VEX files 
and places it in the parameter store for later fringe fitting operations. It handles 
clock model extraction, sample rate determination, and other unit conversions.
Key functions include `extract_clock_model()` for clock information extraction and 
`extract_vex_info()` for additional meta-data. 
