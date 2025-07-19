HDF5 Data Export
~~~~~~~~~~~~~~~~

The HDF5 plugin provides some data export capabilities for converting 
HOPS4 internal data structures to the HDF5 format. The output HDF5 file structure 
should **NOT** be considered stable until it has been evaluated for inter-operability 
with other tools and evaluated for long-term data archival. There is currently *NO* import
capability -- to read HDF5 data back into the HOPS4 format.

**HDF5 Export Applications:**
The HDF5 plugin includes a command-line application for data conversion:

**hops2hdf5 Application:**
- Converts HOPS4 files to HDF5 format
- Supports batch processing of multiple files
- Configurable output organization


:hops:`MHO_HDF5TypeCode`
------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_HDF5TypeCode`
Template Parameters                             XDataType (C++ data type to map)
Primary Functionality                           Template class for HDF5 type code determination
Key Features                                    | Type mapping for fundamental C++ types to HDF5 types
                                                | Complex number support (std::complex<float/double>)
                                                | HDF5 compound type creation for complex numbers
                                                | Automatic type code generation
=============================================== ====================================================================

The :hops:`MHO_HDF5TypeCode` class provides template-based type mapping between 
C++ data types and their corresponding HDF5 type codes. It supports all fundamental 
types used in VLBI data processing and including handling for complex numbers.

**Supported Types:**
- Signed integers: int8_t, int16_t, int32_t, int64_t
- Unsigned integers: uint8_t, uint16_t, uint32_t, uint64_t
- Floating-point: float, double, long double
- Complex numbers: std::complex<float>, std::complex<double>
- Boolean and character types

:hops:`MHO_HDF5Datasets`
------------------------

=============================================== ====================================================================
File                                            MHO_HDF5Datasets.hh
Primary Functionality                           Template functions for HDF5 dataset creation
Key Features                                    | Scalar dataset creation with automatic type mapping
                                                | N-dimensional array dataset creation
                                                | Dimension scale creation for coordinate axes
                                                | String dataset support with variable-length strings
                                                | Metadata attribute attachment from JSON objects
=============================================== ====================================================================

The MHO_HDF5Datasets header provides a collection of template functions 
for creating various types of HDF5 datasets from HOPS4 data structures.

**Dataset Types Supported:**
- Scalar datasets with automatic HDF5 type mapping
- Multi-dimensional arrays with dimension scales
- String datasets with variable-length encoding
- Coordinate axis datasets with proper labeling
- Metadata tagged datasets with JSON attribute conversion (to attributes or string pack)

:hops:`MHO_HDF5Attributes`
--------------------------

=============================================== ====================================================================
File                                            MHO_HDF5Attributes.hh
Template Parameters                             Various template functions with different attribute types
Primary Functionality                           Template functions for HDF5 attribute creation
Key Features                                    | Scalar attribute creation with type mapping
                                                | Vector attribute creation for array metadata
                                                | String attribute specializations
                                                | JSON object to HDF5 attribute conversion
=============================================== ====================================================================

The MHO_HDF5Attributes header provides template functions for creating HDF5 
attributes that store metadata associated with datasets and groups.

**Attribute Types:**
- Scalar attributes for individual metadata values
- Vector attributes for array-based metadata
- String attributes with proper encoding
- JSON (serialized to string) for complex metadata structures

:hops:`MHO_ContainerHDF5Converter`
----------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ContainerHDF5Converter`
Template Parameters                             XContainerType (container type to convert)
Primary Functionality                           Converts ndarray-based containers to HDF5 representation
Key Features                                    | SFINAE template specialization mechanism
                                                | Scalar, vector, axis, and table container support
                                                | Automatic axis dumping with metadata preservation
                                                | JSON metadata to HDF5 attributes conversion
                                                | Export-only functionality
=============================================== ====================================================================

The :hops:`MHO_ContainerHDF5Converter` class provides the core functionality for 
converting HOPS4 container objects to HDF5 format. It uses SFINAE to handle different 
container types appropriately.

**Container Types Supported:**
- Scalar containers (single values with metadata)
- Vector containers (1D arrays with coordinate axes)
- Axis containers (coordinate axis definitions)
- Table containers (multi-dimensional arrays with axes)

**Conversion Features:**
- Automatic metadata preservation as HDF5 attributes
- Coordinate axis information retention
- Proper handling of complex number data
- JSON metadata conversion to HDF5 attributes or serialized strings

:hops:`MHO_HDF5ContainerFileInterface`
--------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_HDF5ContainerFileInterface`
Primary Functionality                           Converts complete HOPS4 files to HDF5 format
Key Features                                    | Inherits from MHO_ContainerFileInterface
                                                | Inherits from MHO_HDF5ConverterDictionary
                                                | Complete data store to HDF5 conversion
=============================================== ====================================================================

The :hops:`MHO_HDF5ContainerFileInterface` class provides high-level functionality 
for converting entire HOPS4 files to HDF5 format. It orchestrates the conversion 
of all data stores and maintains proper hierarchical organization.

**File Conversion Features:**
- Complete fringe file conversion to HDF5
- Preservation of all data stores (parameters, containers, scan data)
- Hierarchical group organization
- Configurable naming conventions
- Metadata preservation throughout conversion

:hops:`MHO_HDF5ConverterDictionary`
-----------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_HDF5ConverterDictionary`
Primary Functionality                           Manages HDF5 converter registry and lookup
Key Features                                    | Converter registration and management
                                                | Type-based converter lookup
                                                | Extensible converter architecture
                                                | Support for custom converter implementations
=============================================== ====================================================================

The :hops:`MHO_HDF5ConverterDictionary` class manages the registry of available 
HDF5 converters and provides lookup functionality for appropriate converters 
based on data types (see the implementation of :hops:`MHO_ContainerJSONConverter` upon which it is based).

