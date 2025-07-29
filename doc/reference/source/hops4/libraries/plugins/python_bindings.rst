Python Bindings
~~~~~~~~~~~~~~~


The Python bindings plugin provides a set of features for integrating the HOPS4 C++ libraries with Python. 
This plugin enables Python access to HOPS4 data structures, operators, and processing. This enables custom 
user analysis scripts (written in python) to be injected into the fringe-fitter control flow and execution.

It uses pybind11 for creating language bindings and supports a numpy ND-array 
interface for direct access to MHO_TableContainer (array) data. Type safety is 
handled through automatic type checking and conversion. 
Error handling allows exceptions to pass from Python to a C++ caller and be caught.

For performance, the system supports zero-copy operations when possible, allowing direct memory access to MHO_TableContainer array 
data (but not meta-data or axis-data). Data transfer between C++ and Python is designed to minimize copying. 
On-demand loading is used to load data structures only when required.

Use cases for this library include custom calibration and data treatment with user Python scripts, 
algorithm prototyping and user supplied visualization through custom plotting routines.


:hops:`MHO_PyTableContainer`
----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PyTableContainer`
Template Parameters                             XTableType (table container type)
Primary Functionality                           Python bindings for template MHO_TableContainer objects
Key Features                                    | Copy-free numpy array access to C++ array data
                                                | Coordinate axis access as Python lists
                                                | Metadata management via Python dictionaries
                                                | Type-safe axis label modification
=============================================== ====================================================================

The :hops:`MHO_PyTableContainer` class provides Python bindings for HOPS4 table 
containers, and allows efficient access to multi-dimensional data arrays via a 
Numpy interface.

**Python Interface Features:**
    - **Zero-copy Access**: Direct access to C++ memory through NumPy arrays
    - **Metadata Integration**: Python dictionary access to container metadata
    - **Axis Management**: Python list access to coordinate axis labels
    - **Type Safety**: Maintains type integrity across Python/C++ boundary
    - **Performance**: Efficient data access without memory copying

:hops:`MHO_PyContainerStoreInterface`
-------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PyContainerStoreInterface`
Primary Functionality                           Python binding to the MHO_ContainerStore
Key Features                                    | UUID-based object retrieval from Python side
                                                | Type-safe container access with automatic casting
                                                | Object listing and metadata queries
                                                | Extension-based Python wrapper creation
=============================================== ====================================================================

The :hops:`MHO_PyContainerStoreInterface` class provides Python access to the 
HOPS4 container store, allowing Python scripts to retrieve and manipulate 
data containers created during fringe fitting.

**Container Store Features:**
    - **UUID Lookup**: Retrieve containers by unique identifiers
    - **Type Safety**: Automatic type checking and casting
    - **Object Enumeration**: Lists all containers and their metadata
    - **Dynamic Wrapping**: Create Python wrappers on demand (invisible to python user)

:hops:`MHO_PyParameterStoreInterface`
-------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PyParameterStoreInterface`
Primary Functionality                           Python bindings for the MHO_ParameterStore
Key Features                                    | Path-based parameter access from Python
                                                | JSON-to-Python dictionary object conversion
                                                | Complete parameter store dump functionality
                                                | Type-aware parameter retrieval
=============================================== ====================================================================

The :hops:`MHO_PyParameterStoreInterface` class provides Python access to the 
HOPS4 parameter store, enabling Python scripts to read and modify processing 
parameters during fringe fitting operations.

**Parameter Access Features:**
    - **Path-based Access**: Hierarchical parameter access using path strings
    - **JSON Integration**: Automatic conversion between JSON and Python objects
    - **Type Preservation**: Maintains parameter types across language boundaries
    - **Bulk Operations**: Complete parameter store dumping and loading

:hops:`MHO_PyScanStoreInterface`
--------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PyScanStoreInterface`
Primary Functionality                           Python bindings for the MHO_ScanDataStore
Key Features                                    | Directory-based scan data loading
                                                | Baseline, station, and fringe file management
                                                | Lazy loading of data containers
                                                | Root/OVEX file data access
=============================================== ====================================================================

The :hops:`MHO_PyScanStoreInterface` class provides Python access to VLBI scan 
data, enabling Python scripts to load and process/inspect multiple observational data files.

**Scan Data Features:**
    - **File Management**: Handles loading and memory management of .cor, .frng, and root files
    - **Lazy Loading**: Efficient memory usage through on-demand data loading and caching
    - **Metadata Access**: Station, baseline, and observation metadata
    - **VEX Integration**: Access to experiment and observation parameters

:hops:`MHO_PyFringeDataInterface`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PyFringeDataInterface`
Primary Functionality                           Python access to fringe data during processing
Key Features                                    | Direct access to visibility data arrays
                                                | Frequency and time axis information
                                                | Baseline and polarization metadata
                                                | Real-time data modification capabilities
=============================================== ====================================================================

The :hops:`MHO_PyFringeDataInterface` class provides Python access to the in-memory 
fringe data being processed (by the fringe fitter). This allows Python scripts to inspect and modify 
visibility data during fringe fitting operations.

:hops:`MHO_PyGenericOperator`
-----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PyGenericOperator`
Primary Functionality                           Injects Python functions into fringe fitter control flow
Key Features                                    | Full access to fringe data, container store, parameter store
                                                | Configuration via Python module and function name 
                                                | Exception handling for Python errors
                                                | Integration with HOPS4 operator framework
=============================================== ====================================================================

The :hops:`MHO_PyGenericOperator` class enables the injection of Python functions 
into the HOPS4 fringe fitting pipeline, providing a mechanism for custom processing and analysis.

**Operator Integration Features:**
    - **Full Data Access**: Python functions can access all HOPS4 data structures via MHO_PyFringeDataInterface.
    - **Pipeline Integration**: Integration with existing operator framework
    - **Error Handling**: Exception handling for Python errors
    - **Configuration**: Configurable module and function names to select python functionas at run time

:hops:`MHO_PyUnaryOperator` and :hops:`MHO_PyUnaryTableOperator`
----------------------------------------------------------------

=============================================== ====================================================================
Class Family                                    :hops:`MHO_PyUnaryOperator`, :hops:`MHO_PyUnaryTableOperator`
Template Parameters                             Various template parameters for data types
Primary Functionality                           Python bindings for unary operator base classes
Key Features                                    | Template-based operator bindings
                                                | Type-safe operator inheritance
                                                | Python implementation of C++ operator patterns
                                                | Support for custom operator development
=============================================== ====================================================================

These classes provide Python bindings for the unary operator base classes, 
enabling Python developers to create custom operators that integrate with 
with the HOPS4 processing pipeline.

:hops:`MHO_PythonOperatorBuilder`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PythonOperatorBuilder`
Primary Functionality                           Builder for Python-based operators
Key Features                                    | Constructs Python operator instances
                                                | Integrates with operator builder framework
                                                | Configurable Python module loading
                                                | Type-safe operator construction
=============================================== ====================================================================

The :hops:`MHO_PythonOperatorBuilder` class provides a builder for creating 
Python-based operators that can be integrated into the HOPS4 processing pipeline.

**Python Module Integration:**
The Python bindings are organized into several modules (available via import):
    - **mho_containers**: Container and data access functionality
    - **mho_operators**: Operator framework and custom operator support
    - **mho_parameters**: Parameter management and configuration
    - **mho_scan_data**: Scan data loading and management
