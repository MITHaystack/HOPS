Miscellaneous Utilities
~~~~~~~~~~~~~~~~~~~~~~~~

The utilities library includes various support classes for general-purpose 
operations. These tools provide supporting functionality for the HOPS4 framework's 
data processing, file I/O, time handling, and system integration requirements.


:hops:`MHO_Tokenizer`
---------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_Tokenizer`
Category                                        Miscellaneous
Configuration Parameters                        | Delimiter strings (default: space)
                                                | Empty token inclusion
                                                | Multi-character delimiter support
                                                | Quote preservation
                                                | Whitespace trimming
Primary Functionality                           Configurable string tokenization
Key Features                                    | Flexible delimiter configuration
                                                | Quote-aware parsing
                                                | Whitespace handling options
                                                | Empty token control
=============================================== ====================================================================

The :hops:`MHO_Tokenizer` class provides flexible string tokenization with 
configurable delimiters and parsing options including quote preservation and 
whitespace handling. This utility class is heavily used by the control file parsing 
library and vex parsing library. 

:hops:`MHO_UUID`
----------------

=============================================== ====================================================================
Class                                           :hops:`MHO_UUID`
Category                                        Miscellaneous
Primary Functionality                           128-bit UUID storage and manipulation
Key Features                                    | 16-byte UUID representation
                                                | String and integer conversion
                                                | Comparison operators
                                                | Support for stream operations
=============================================== ====================================================================

The :hops:`MHO_UUID` class provides 128-bit UUID storage and manipulation 
with some conversion capabilities and comparison operators for object identification.

:hops:`MHO_UUIDGenerator`
-------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_UUIDGenerator`
Category                                        Miscellaneous
Primary Functionality                           RFC 4122 compliant UUID generation
Key Features                                    | Pseudo-random UUID generation
                                                | Follows RFC 4122 (v4) standard
                                                | String conversion utilities
=============================================== ====================================================================

The :hops:`MHO_UUIDGenerator` class generates a RFC-4122 (v4) compliant 
pseudo-random UUID for object identification. The random number generation 
is done via the STL Mersenne Twister algorithm.

:hops:`MHO_MD5HashGenerator`
----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_MD5HashGenerator`
Category                                        Miscellaneous
Template Parameters                             Variadic template for supported data types
Primary Functionality                           MD5 hash generation for data integrity
Key Features                                    | Streaming MD5 hash computation
                                                | Support for POD types and strings
                                                | UUID conversion from hash digests
=============================================== ====================================================================

The :hops:`MHO_MD5HashGenerator` class provides MD5 hash generation capabilities
for data hasing/UUID creation. It has streaming support for various data types.
The underlying implementation is done by the picohash.h header library. placed 
under public domain by Kazuho Oku.

Additional Utility Classes
~~~~~~~~~~~~~~~~~~~~~~~~~~

The utilities library also includes several specialized utility classes/files:

- :hops:`MHO_BidirectionalIterator` and :hops:`MHO_BidirectionalStrideIterator` - Iterator implementations for container traversal
- :hops:`MHO_Constants` - Basic mathematical and physical constants
- MHO_EncodeDecodeValue.hh - Value encoding and decoding utilities
- :hops:`MHO_ExtensibleElement` - Base class for extensible data structures
- :hops:`MHO_IndexLabelInterface` and :hops:`MHO_IntervalLabelInterface` - meta-data labeling
- :hops:`MHO_Interval` - Interval representation and manipulation
- :hops:`MHO_JSONHeaderWrapper` - JSON header wrapper for data serialization (wraps the nlohmann::json head-only library)
- :hops:`MHO_MPIInterface` and MHO_MPIInterfaceWrapper.hh - basic MPI environment initialization/interaction
- :hops:`MHO_NumpyTypeCode` - Numpy type code creation
- :hops:`MHO_ParameterStore` - Parameter storage and retrieval

- :hops:`MHO_Unit` - Unit representation and conversion - NOT YET IMPLEMENTED
- picohash.h - Lightweight hashing library placed under public domain by Kazuho Oku.
