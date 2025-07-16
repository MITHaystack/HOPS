File Operation Utilities
~~~~~~~~~~~~~~~~~~~~~~~~~

The utilities library provides a collection of file and directory manipulation 
tools for data I/O operations with HOPS4 objects.

:hops:`MHO_BinaryFileInterface`
-------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_BinaryFileInterface`
Category                                        File Operations
Template Parameters                             Templated by serializable object types
Configuration Parameters                        Optional .index file collection (stores object file keys/offfsets)
Primary Functionality                           High-level binary file I/O with object serialization
Key Features                                    | Automatic object key generation and validation
                                                | Optional .index file creation for faster access
                                                | UUID-based type/object identification
=============================================== ====================================================================

The :hops:`MHO_BinaryFileInterface` class provides read/write functionality on
binary files. These operations handle object serialization, key generation, and 
optional .index file creation for (later) efficient data access.

:hops:`MHO_BinaryFileStreamer`
------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_BinaryFileStreamer`
Category                                        File Operations
Template Parameters                             Variadic template for supported data types
Primary Functionality                           Low-level binary streaming for POD and JSON types
Key Features                                    | Support for POD types, strings, and JSON objects
                                                | Byte count tracking for written data
                                                | CBOR encoding for JSON data
=============================================== ====================================================================

The :hops:`MHO_BinaryFileStreamer` class provides low-level binary streaming 
capabilities with support for plain-old-data types, strings, 
and JSON objects (using CBOR encoding). It is used by MHO_BinaryFileInterface.

:hops:`MHO_DirectoryInterface`
------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_DirectoryInterface`
Category                                        File Operations
Primary Functionality                           Directory and file system operations
Key Features                                    | Directory traversal and file listing
                                                | File filtering by extension and prefix
                                                | Legacy MK4 file format identification
                                                | Path manipulation utilities
=============================================== ====================================================================

The :hops:`MHO_DirectoryInterface` class provides several directory and file 
system operations. Primarily it is responsible for directory traversal, 
file listing, filtering, and identification. Identification of legacy MK4 
file formats is alos supported.

:hops:`MHO_FileStreamer`
------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FileStreamer`
Category                                        File Operations
Primary Functionality                           Base class for file streaming operations
Key Features                                    | Abstract interface for file I/O operations
                                                | File state management (readable/writable/closed)
                                                | Buffered I/O (2MB buffer size)
                                                | Object state tracking for error handling
=============================================== ====================================================================

The :hops:`MHO_FileStreamer` class provides a base interface for file streaming operations 
with state management and buffered I/O capabilities.

:hops:`MHO_LockFileHandler`
---------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_LockFileHandler`
Category                                        File Operations
Configuration Parameters                        Legacy mode support (enabled by default)
Primary Functionality                           File locking mechanism for concurrent access control
Key Features                                    | Singleton pattern implementation
                                                | Legacy type_2xx file naming convention support
                                                | Signal handling for cleanup on termination
                                                | Process priority-based lock resolution
=============================================== ====================================================================

The :hops:`MHO_LockFileHandler` class implements a file locking mechanism to 
prevent concurrent nameing conflicts during write operations. This mechanism 
supports both legacy (Mk4) and HOPS4 (fringe) file naming conventions.


:hops:`MHO_FileKey`
-------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FileKey`
Category                                        File Operations
Primary Functionality                           Object description and file location
Key Features                                    64 byte file object key
=============================================== ====================================================================

The :hops:`MHO_FileKey` class stores object type and UUID information, as well 
as object size and *shortname* meta-data. For every HOPS4 object serialized to file 
it precedes the object binary data. This provides a quick indexing method into 
binary file data, and allows the file I/O mechanism to skip over unrecognized objects.


:hops:`MHO_Serializable`
------------------------ - 

=============================================== ====================================================================
Class                                           :hops:`MHO_Serializable`
Category                                        File Operations
Primary Functionality                           Base class for all serializable objects
Key Features                                    Mechanisms for object version control and identification
=============================================== ====================================================================

The :hops:`MHO_Serializable` class provides an interface required by the file 
serialization library to read/write objects to disk. It provides a mechanism to
identify objects and switch behavior based on object version. Classes which
inherit from :hops:`MHO_Serializable` must provide a method to calculate the size
(in bytes) of their serialized data.
