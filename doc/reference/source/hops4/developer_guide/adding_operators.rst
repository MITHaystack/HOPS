===============================================================================
HOPS Developer Guide: Adding New Parameters and Calibration Operators
===============================================================================

This document describes the end-to-end pattern for adding (1) a new parameter
to the MHO_ParameterStore for downstream/internal consumption, and (2) a new calibration
(C++, compiled) operator applied to visibility data.

.. contents:: Table of Contents
   :depth: 1
   :local:


1. Architecture Overview
========================

The HOPS data processing pipeline is driven by a text-based control file,
whose keywords are described by JSON descriptors. The JSON format descriptors live in:

- ``source/cpp_src/Control/format/control/`` (user-accessible keywords)
- ``source/cpp_src/Control/format/control_extensions/`` (advanced/extension keywords)

The data processing pipeline flow consists of:

**Control File** --> **ParameterStore** --> **OperatorBuilders** --> **Operators**

- **Control File**: text keywords and conditionals
- **ParameterStore**: path-based JSON descriptors
- **OperatorBuilders**: construct and configure operator objects
- **Operators**: initialize and execute processing steps (applied to data)

The key library modules involved are:

- **Control/** : Parses the control files, evaluates conditionals, tokenizes
  arguments into an intermediate JSON structure using the format descriptors.
- **Initialization/** : Contains MHO_OperatorBuilder subclasses that construct
  operator instances, configures them with data/parameters/control statements,
  and registers them with the MHO_OperatorToolbox.
- **Calibration/** : Contains the actual operator implementations (i.e. the code
  that transforms visibility data).
- **Operators/** : Base class hierarchy (MHO_Operator, MHO_UnaryOperator<>,
  MHO_BinaryOperator<>, MHO_InspectingOperator<>) from which the Calibration operators are derived.
- **Utilities/** : MHO_ParameterStore, MHO_ContainerStore (data containers for visiblity, etc.).


2. The Data Flow Pipeline
=========================

Below is a rough decription of the step-by-step data flow for a single scan/baseline when processed
via fourfit4:

1. **MHO_ControlFileParser::ParseControl()**

   - Reads the raw control file, strips comments, tokenizes statements
   - Discovers keywords by scanning JSON filenames in ``format/control/``
   - Groups tokens into MHO_ControlStatement objects (one per keyword)

2. **MHO_ControlElementParser::ParseControlStatement()**

   - For each statement, looks up its JSON format descriptor
   - Dispatches tokens to MHO_ControlTokenProcessor based on type
     (int, real, string, bool, list_int, list_real, list_string,
     fixed_length_list_string, compound)
   - Produces a structured JSON "control block" value for each statement

3. **MHO_ControlConditionEvaluator::GetApplicableStatements()**

   - Evaluates conditional statements like "if station X and source Y" on a per-pass basis.
   - Returns only the control blocks whose conditions are true

4. **MHO_ParameterManager::ConfigureAll()**

   - Iterates all statements with ``statement_type == "parameter"``
   - Calls MHO_ParameterConfigurator::Configure() to store values in
     MHO_ParameterStore at paths like:

     - ``/control/config/<name>``
     - ``/control/station/<STATION_CODE>/<name>``
     - ``/control/fit/<name>``
     - ``/control/global/<name>``

   - Once stored, all parameter statements are removed from further processing

5. **MHO_OperatorBuilderManager::BuildOperatorCategory(category)**

   - For each category: "labeling", "selection", "flagging",
     "calibration", "prefit", "postfit", "finalize"
   - Iterates over the applicable control statements that match the category
   - For each statement, finds the registered builder by name
   - Calls ``builder->SetConditions(block)``, ``SetAttributes(stmt)``, ``Build()``

6. **Each MHO_OperatorBuilder::Build()**

   - Validates the data/control statement configuration (IsConfigurationOk)
   - Extracts parameters from fAttributes and/or the fParameterStore
   - Retrieves data arrays from fContainerStore
   - Constructs the operator, calls SetArgs() to pass in the data pointers and configures it
   - Registers with ``fOperatorToolbox->AddOperator(op, name, category)``

7. **MHO_FringeFitter then executes operators ordered by category/priority**

   - Calls ``operator->Initialize()`` then ``operator->Execute()`` for each operator in the pipeline
   - Operators may modify visibility/weight data in-place or out-of-place


3. Adding a New Parameter (MHO_ParameterStore)
==============================================

A "parameter" is a control file keyword whose sole purpose is to store a value
in the MHO_ParameterStore for later consumption by builders or operators.

Example: ``ref_freq 14000.0`` stores 14000.0 at ``/control/config/ref_freq``

If you wish to add a new parameter value that can be consumed within the C++ application (or plugins),
you will need to:

STEP 3.1 - Create the JSON format descriptor
---------------------------------------------

File: ``source/cpp_src/Control/format/control/<name>.json``

Required fields:

- ``"name"``: the keyword name (must match filename without .json)
- ``"statement_type"``: ``"parameter"``
- ``"type"``: one of the supported value types (see `5. JSON Format Descriptor Reference`_)

Optional fields:

- ``"parameter_type"``: one of: ``"config"`` (default), ``"station"``, ``"global"``,
  ``"baseline"``, ``"fit"``, ``"plot"``

Example - simple scalar parameter:

.. code-block:: json

   {
       "name": "new_param",
       "statement_type": "parameter",
       "parameter_type": "config",
       "type": "real"
   }

Example - per-station string parameter:

.. code-block:: json

   {
       "name": "mount_type",
       "statement_type": "parameter",
       "parameter_type": "station",
       "type": "string"
   }

Example - list parameter:

.. code-block:: json

   {
       "name": "new_freq_list",
       "statement_type": "parameter",
       "parameter_type": "config",
       "type": "list_real"
   }

No C++ code changes are needed to add simple parameters to the control file syntax, as long as
they belong to the class of value types that are already allowed (see `5. JSON Format Descriptor Reference`_).
The MHO_ParameterConfigurator handles all supported types generically, and will insert them in the parameter 
store in the locations specified by the format descriptor.

STEP 3.2 - Storage paths
-------------------------

Parameters are stored in MHO_ParameterStore at deterministic paths:

.. list-table::
   :header-rows: 1
   :widths: 20 50

   * - Parameter type
     - Path pattern
   * - ``"config"``
     - ``/control/config/<name>``
   * - ``"global"``
     - ``/control/global/<name>``
   * - ``"fit"``
     - ``/control/fit/<name>``
   * - ``"plot"``
     - ``/control/plot/<name>``
   * - ``"baseline"``
     - ``/control/baseline/<name>``
   * - ``"station"``
     - ``/control/station/<STATION_CODE>/<name>``

For ``"station"`` parameters: STATION_CODE is the 2-char site code, e.g. "Gs", "Wf".
If used with "if station X" conditionals, the station code is resolved from the
condition token. Multiple condition blocks may produce multiple paths.

STEP 3.3 - Consuming the parameter downstream
----------------------------------------------

In a builder (preferably), retrieve the parameter from the store via:

.. code-block:: cpp

   // Check presence first (recommended):
   if (fParameterStore->IsPresent("/control/config/new_param")) {
       double val = fParameterStore->GetAs<double>("/control/config/new_param");
       // use val...
   }

   // Or retrieve with default (returns 0.0 for double, "" for string, etc.):
   double val = fParameterStore->GetAs<double>("/control/config/new_param");

   // Or retrieve by reference (returns true if found):
   double val;
   bool ok = fParameterStore->Get("/control/config/new_param", val);

For per-station parameters the exact path is dictated by the site_id, so the 
station 2-char code must be known or retrieved first:

.. code-block:: cpp

   std::string ref_id = fParameterStore->GetAs<std::string>("/ref_station/site_id");
   std::string path = "/control/station/" + ref_id + "/mount_type";
   std::string mount = fParameterStore->GetAs<std::string>(path);
   
Note: Parameter store access should be limited to the operator build classes (not the operators themselves). 
Individual parameters should be passed to operators indirectly via setter/getters to avoid coupling them to the parameter store
and making unit testing more difficult.

STEP 3.4 - No CMake changes needed
-----------------------------------

JSON format files are auto-discovered via:

.. code-block:: cmake

   file(GLOB_RECURSE FORMAT_FILES ... "*.json")

Simply placing the file in ``format/control/`` or ``format/control_extensions/`` then running ``make install``
is sufficient. Files are installed to ``DATA_INSTALL_DIR/control/`` at build time.


4. Adding a New Calibration Operator
=====================================

An "operator" is a control file keyword that causes a data transformation to
be constructed, configured, and registered with the MHO_OperatorToolbox.

The operator must be a C++ class inheriting from an operator base class, and
the builder must be a C++ class inheriting from MHO_OperatorBuilder. You will need both 
the operator and the operator builder (along with a JSON format descriptor to trigger it from the control file).
The operator build must be registered with the MHO_OperatorBuilderManager.

STEP 4.1 - Choose the operator base class
------------------------------------------

The base class depends on how your operator accesses data:

.. list-table::
   :header-rows: 1
   :widths: 30 50

   * - Base class
     - Use case
   * - ``MHO_UnaryOperator<T>``
     - Most common. Single input array, operates in-place. Input = output type.
       Template parameter T is the data type (e.g. visibility_type, weight_type).
       Must implement: ``ExecuteInPlace(T* in)``
   * - ``MHO_BinaryOperator<A1, A2, A3>``
     - Two inputs, one output. Different types allowed. E.g. visibility + weight -> SBD.
       Must implement: ``ExecuteImpl(const A1*, const A2*, A3*)``
   * - ``MHO_InspectingOperator<T>``
     - Read-only inspection of a single const array. Produces results in an internal
       workspace or parameter store (example: MHO_MBDelaySearch).
       Must implement: ``InitializeImpl(const T* in)`` and ``ExecuteImpl(const T* in)``
   * - ``MHO_TransformingOperator<A1, A2>``
     - Transforms one array type into a different type. E.g. 3D -> 4D.
       Must implement: ``ExecuteImpl(const A1*, A2*)``
   * - ``MHO_Operator`` (direct)
     - Not commonly used. No template helpers. You must manage all arguments via custom setters.
       Must implement: ``Initialize()`` and ``Execute()``

Common data types used be operators (from ``MHO_ContainerDefinitions.hh``):

- **visibility_type** : 4D complex [polprod, channel, time, freq]
- **weight_type** : 4D real [polprod, channel, time, freq] (note: freq axis has size 1)
- **sbd_type** : 4D complex (single-band-delay workspace)
- **station_coord_type** : station coordinate data

The large majority of (>90%) of calibration operators, inherit from ``MHO_UnaryOperator<visibility_type>``.

STEP 4.2 - Write the operator header
-------------------------------------

File: ``source/cpp_src/Calibration/include/MHO_NewOperator.hh``

Pattern (UnaryOperator, in-place):

.. code-block:: cpp

   #ifndef MHO_NewOperator_HH__
   #define MHO_NewOperator_HH__

   #include "MHO_ContainerDefinitions.hh"
   #include "MHO_UnaryOperator.hh"

   namespace hops
   {
   class MHO_NewOperator: public MHO_UnaryOperator< visibility_type >
   {
       public:
           MHO_NewOperator();
           virtual ~MHO_NewOperator();

           // Configure operator (called from builder)
           void SetSomeParameter(double val) { fParam = val; }

       protected:
           // Pure virtual - must implement
           virtual bool ExecuteInPlace(visibility_type* in) override;

           // Optional - override if you need per-scan initialization
           // virtual bool InitializeInPlace(visibility_type* in) override;

       private:
           double fParam;
   };
   } // namespace hops
   #endif

Pattern (InspectingOperator):

.. code-block:: cpp

   class MHO_NewInspector: public MHO_InspectingOperator< visibility_type >
   {
       protected:
           virtual bool InitializeImpl(const visibility_type* in) override;
           virtual bool ExecuteImpl(const visibility_type* in) override;
   };

STEP 4.3 - Write the operator source
-------------------------------------

File: ``source/cpp_src/Calibration/src/MHO_NewOperator.cc``

Pattern:

.. code-block:: cpp

   #include "MHO_NewOperator.hh"

   namespace hops
   {
   MHO_NewOperator::MHO_NewOperator() : fParam(0.0) {}
   MHO_NewOperator::~MHO_NewOperator() {}

   bool MHO_NewOperator::ExecuteInPlace(visibility_type* in)
   {
       // Access axes:
       auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));
       auto time_ax = &(std::get< TIME_AXIS >(*in));
       auto freq_ax = &(std::get< FREQ_AXIS >(*in));
       auto pol_ax  = &(std::get< POLPROD_AXIS >(*in));

       // Iterate and modify data in-place:
       for (std::size_t ch = 0; ch < chan_ax->GetSize(); ch++) {
           for (std::size_t t = 0; t < time_ax->GetSize(); t++) {
               // SliceView for specific indices
               in->SliceView(":", ch, t, ":") *= fParam;
           }
       }

       msg_debug("calibration", "applied new operator correction" << eom);
       return true;
   }
   } // namespace hops

Accessing axis labels:

.. code-block:: cpp

   std::string label;
   chan_ax->RetrieveIndexLabel(ch, label);
   chan_ax->RetrieveIndexLabelKeyValue(ch, "key_name", label);

STEP 4.4 - Write the builder header
------------------------------------

File: ``source/cpp_src/Initialization/include/MHO_NewOperatorBuilder.hh``

Pattern:

.. code-block:: cpp

   #ifndef MHO_NewOperatorBuilder_HH__
   #define MHO_NewOperatorBuilder_HH__

   #include "MHO_OperatorBuilder.hh"

   namespace hops
   {
   class MHO_NewOperatorBuilder: public MHO_OperatorBuilder
   {
       public:
           MHO_NewOperatorBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
               : MHO_OperatorBuilder(toolbox, fdata) {}

           MHO_NewOperatorBuilder(MHO_OperatorToolbox* toolbox,
                                  MHO_ContainerStore* cstore = nullptr,
                                  MHO_ParameterStore* pstore = nullptr)
               : MHO_OperatorBuilder(toolbox, cstore, pstore) {}

           virtual ~MHO_NewOperatorBuilder() {}

           virtual bool Build() override;
   };
   } // namespace hops
   #endif

.. note::

   Always provide both constructors. The second is needed for unit testing.

STEP 4.5 - Write the builder source
------------------------------------

File: ``source/cpp_src/Initialization/src/MHO_NewOperatorBuilder.cc``

Pattern:

.. code-block:: cpp

   #include "MHO_NewOperatorBuilder.hh"
   #include "MHO_NewOperator.hh"

   #include <memory>

   namespace hops
   {
   bool MHO_NewOperatorBuilder::Build()
   {
       if (!IsConfigurationOk())
           return false;

       msg_debug("initialization", "building new_operator." << eom);

       // 1. Extract operator metadata from fFormat and fAttributes
       std::string op_name    = fAttributes["name"].get<std::string>();
       std::string op_category = "calibration";  // or from fFormat
       double priority        = fFormat["priority"].get<double>();

       // 2. Extract parameters from fAttributes (for compound types, use
       //    fAttributes["value"]["field_name"]) or from fParameterStore
       double some_param = 0.0;
       if (fParameterStore->IsPresent("/control/config/some_param")) {
           some_param = fParameterStore->GetAs<double>("/control/config/some_param");
       }

       // For compound operators, parameters come from fAttributes["value"]:
       // std::string algo = fAttributes["value"]["algorithm_type"].get<std::string>();

       // 3. Retrieve data containers from fContainerStore
       visibility_type* vis_data =
           fContainerStore->GetObject<visibility_type>("vis");
       if (vis_data == nullptr) {
           msg_error("initialization",
                     "cannot construct MHO_NewOperator without visibility data."
                     << eom);
           return false;
       }

       // 4. Construct and configure the operator
       std::unique_ptr<MHO_NewOperator> op(new MHO_NewOperator());
       op->SetArgs(vis_data);
       op->SetSomeParameter(some_param);

       // Optional: station-specific application
       // op->SetStationIdentifiers(GetMatchingStationIdentifiers());

       // 5. Register with the toolbox
       op->SetName(op_name);
       op->SetPriority(priority);

       bool replace_duplicates = false; // true if operator can be re-added per pass
       this->fOperatorToolbox->AddOperator(
           std::move(op), op_name, op_category, replace_duplicates);

       return true;
   }
   } // namespace hops

Builder accessible members in the MHO_OperatorBuilder base class that you may use
when constructing and operators are:

- **fOperatorToolbox** : ``MHO_OperatorToolbox*`` (register operators when complete here)
- **fContainerStore** : ``MHO_ContainerStore*`` (retrieve data arrays)
- **fParameterStore** : ``MHO_ParameterStore*`` (retrieve config parameters)
- **fFormat** : ``mho_json`` (the JSON format descriptor)
- **fAttributes** : ``mho_json`` (the parsed control statement)
- **fConditions** : ``mho_json`` (the conditional block tokens)
- **GetMatchingStationIdentifiers()** : returns matched station IDs for this baseline

STEP 4.6 - Create the JSON format descriptor
---------------------------------------------

File: ``source/cpp_src/Control/format/control/<name>.json``

For simple operator types (int, real, string, bool, list_\*, fixed_length_list_string):

.. code-block:: json

   {
       "name": "new_operator",
       "statement_type": "operator",
       "operator_category": "calibration",
       "type": "real",
       "priority": 3.5
   }

For operators which require multiple fields to be configured, use the compound type:

.. code-block:: json

   {
       "name": "new_operator",
       "statement_type": "operator",
       "operator_category": "calibration",
       "type": "compound",
       "priority": 3.5,
       "parameters": {
           "channel_names": {"type": "string"},
           "correction_values": {"type": "list_real"},
           "!mode": {"type": "string"}
       },
       "fields": [
           "channel_names",
           "correction_values",
           "!mode"
       ]
   }

Notes on compound format:

- ``"parameters"`` defines the type of each sub-field
- ``"fields"`` lists the expected tokens in specific order expected from the control file
- Fields prefixed with ``"!"`` are optional (parser stops consuming tokens
  if an optional field is missing)
- List types (list_int, list_real, list_string) consume all remaining tokens
- Control file usage::

     new_operator chan1,chan2 1.0 2.0 3.0 fast

- In the builder, access compound values via:

  .. code-block:: cpp

     fAttributes["value"]["channel_names"].get<std::string>();
     fAttributes["value"]["correction_values"].get<std::vector<double>>();
     fAttributes["value"]["mode"].get<std::string>();

Operator categories and typical priorities:

.. list-table::
   :header-rows: 1
   :widths: 15 15 30

   * - Category
     - Priority range
     - Purpose
   * - labeling
     - 0.1 - 0.9
     - Channel/polarization labeling
   * - selection
     - 1.0 - 1.9
     - Data selection/filtering
   * - flagging
     - 2.0 - 2.9
     - Flagging bad data
   * - calibration
     - 3.0 - 3.99
     - Phase/delay corrections
   * - prefit
     - 7.0 - 7.9
     - Pre-fitting operations
   * - postfit
     - 8.0 - 8.9
     - Post-fitting operations
   * - finalize
     - 9.0+
     - Final cleanup

STEP 4.7 - Register the builder in MHO_OperatorBuilderManager
--------------------------------------------------------------

File: ``source/cpp_src/Initialization/src/MHO_OperatorBuilderManager.cc``

The last C++ change needed is to register your new operator builder with the manager,
so it can be fired by the control statements. 
In ``CreateDefaultBuilders()``, add:

.. code-block:: cpp

   AddBuilderType< MHO_NewOperatorBuilder >("new_operator", "new_operator");

The first argument is the builder name (used for lookup), the second is the
format key (must match the JSON descriptor name). Both are typically the same.

If the operator is NOT user-accessible (no control file keyword), add it in
``CreateNullFormatBuilders()`` with an inline defined format:

.. code-block:: cpp

   mho_json fmt;
   fmt["name"] = "internal_operator";
   fmt["operator_category"] = "calibration";
   fmt["priority"] = 3.5;
   AddBuilderTypeWithFormat<MHO_NewOperatorBuilder>("internal_operator", fmt);

Also add the ``#include`` for the builder header at the top of the file. This is
not typical, and triggering null-format builders/operators is beyond the scope of this document.


5. JSON Format Descriptor Reference
====================================

The full specification of JSON format descriptor fields is as follows:

Common fields:

- ``"name"`` : string - keyword name, must match filename (required)
- ``"statement_type"`` : string - ``"parameter"``, ``"operator"``, ``"conditional"``,
  ``"deprecated"``, or ``"unknown"`` (required)

For "parameter" statements:

- ``"parameter_type"`` : string - ``"config"``, ``"station"``, ``"global"``, ``"baseline"``,
  ``"fit"``, ``"plot"`` (default: ``"config"``)
- ``"type"`` : value type (see below)

For "operator" statements:

- ``"operator_category"`` : string - ``"labeling"``, ``"selection"``, ``"flagging"``,
  ``"calibration"``, ``"prefit"``, ``"postfit"``, ``"finalize"``
- ``"type"`` : value type (see below)
- ``"priority"`` : double - execution order within category (required)
- ``"parameters"`` : object - sub-field definitions (compound only)
- ``"fields"`` : array - ordered list of field names (compound only)

Supported value types:

.. list-table::
   :header-rows: 1
   :widths: 35 40

   * - Type
     - Control file example
   * - ``"int"``
     - ``keyword 42``
   * - ``"real"``
     - ``keyword 3.14``
   * - ``"string"``
     - ``keyword some_name``
   * - ``"bool"``
     - ``keyword true``
   * - ``"list_int"``
     - ``keyword 1 2 3 4 5``
   * - ``"list_real"``
     - ``keyword 1.0 2.5 3.7``
   * - ``"list_string"``
     - ``keyword A B C D``
   * - ``"fixed_length_list_string"``
     - ``keyword 4 A B C D`` (first token is count)
   * - ``"logical_intersection_list_string"``
     - ``keyword A B C`` (special: merges with previous values taking intersection)
   * - ``"compound"``
     - See compound format above


6. Builder Registration and the Operator Pipeline
=================================================

The MHO_OperatorBuilderManager maintains two maps:

- **fNameToBuilderMap** : name -> builder\* (for keyword lookup)
- **fCategoryToBuilderMap** : category -> builder\* (for category iteration)

Registration happens once at startup via ``CreateDefaultBuilders()``.

Build order (called per scan/baseline):

1. ``BuildOperatorCategory("default")`` - no control input
2. ``BuildOperatorCategory("labeling")`` - channel/pol labeling
3. ``BuildOperatorCategory("selection")`` - data selection
4. ``BuildOperatorCategory("flagging")`` - flag bad data
5. ``BuildOperatorCategory("calibration")`` - phase/delay corrections
6. ``BuildOperatorCategory("prefit")`` - pre-fringe-fitting ops
7. ``BuildOperatorCategory("postfit")`` - post-fringe-fitting ops
8. ``BuildOperatorCategory("finalize")`` - cleanup

Within each category, operators are sorted by priority (ascending).

Conditional evaluation:

The "if station X and source Y" syntax is evaluated per scan. Only
statements whose conditions evaluate to true are passed to builders.
Station matching supports:

- 1-char MK4 IDs (e.g., "E", "G")
- 2-char site codes (e.g., "Wf", "Gs")
- Full station names
- Wildcards: ``"?"`` matches any single-char MK4 ID, ``"??"`` matches all


7. CMake Build System Modifications
====================================

For a new operator in Calibration/:

File: ``source/cpp_src/Calibration/CMakeLists.txt``

Add to ``CALIBRATION_HEADERFILES``:

.. code-block:: cmake

   ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NewOperator.hh

Add to ``CALIBRATION_SOURCEFILES``:

.. code-block:: cmake

   ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_NewOperator.cc

For a new builder in Initialization/:

File: ``source/cpp_src/Initialization/CMakeLists.txt``

Add to ``INITIALIZATION_HEADERFILES``:

.. code-block:: cmake

   ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NewOperatorBuilder.hh

Add to ``INITIALIZATION_SOURCEFILES``:

.. code-block:: cmake

   ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_NewOperatorBuilder.cc

For JSON format descriptors:

No CMake modification needed. Files are auto-discovered via ``GLOB_RECURSE``
in ``source/cpp_src/Control/format/CMakeLists.txt``. Simply place the ``.json``
file in ``format/control/`` or ``format/control_extensions/``. The ``format/control_extensions/`` folder 
is preferred, as the ``format/control/`` directory is reserved for HOPS3 keywords only.


8. Checklist Summary
====================

Adding a NEW PARAMETER:

.. code-block:: text

   [ ] 1. Create source/cpp_src/Control/format/control/<name>.json
        - Set "statement_type": "parameter"
        - Set "parameter_type": "config"|"station"|"global"|"fit"|"plot"|"baseline"
        - Set "type": "int"|"real"|"string"|"bool"|"list_..."
   [ ] 2. (Optional) Update keyword-names.json for documentation
   [ ] 3. No C++ code changes  (other than consumption/retrieval), no CMake changes needed
   [ ] 4. Consume in builder/operator via:
          fParameterStore->GetAs<T>("/control/<type>/<name>")
          or for station params:
          fParameterStore->GetAs<T>("/control/station/<CODE>/<name>")

Adding a NEW OPERATOR (control-file accessible):

.. code-block:: text

   [ ] 1. Create JSON format descriptor:
          source/cpp_src/Control/format/control/<name>.json
          - "statement_type": "operator"
          - "operator_category": "calibration"|"flagging"|...
          - "priority": <double>
          - "type": "real"|"compound"|...
   [ ] 2. Write operator class:
          source/cpp_src/Calibration/include/MHO_NewOperator.hh
          source/cpp_src/Calibration/src/MHO_NewOperator.cc
          - Inherit MHO_UnaryOperator<visibility_type> (or other base)
          - Implement ExecuteInPlace() (or ExecuteImpl())
          - Add setters for configuration parameters
   [ ] 3. Write builder class:
          source/cpp_src/Initialization/include/MHO_NewOperatorBuilder.hh
          source/cpp_src/Initialization/src/MHO_NewOperatorBuilder.cc
          - Inherit MHO_OperatorBuilder
          - Implement Build(): extract params, get data, construct operator,
            register with toolbox
   [ ] 4. Register builder:
          source/cpp_src/Initialization/src/MHO_OperatorBuilderManager.cc
          - Add #include "MHO_NewOperatorBuilder.hh"
          - Add in CreateDefaultBuilders():
            AddBuilderType<MHO_NewOperatorBuilder>("new_op", "new_op")
   [ ] 5. Update CMakeLists.txt:
          source/cpp_src/Calibration/CMakeLists.txt
          - Add header to CALIBRATION_HEADERFILES
          - Add source to CALIBRATION_SOURCEFILES
          source/cpp_src/Initialization/CMakeLists.txt
          - Add header to INITIALIZATION_HEADERFILES
          - Add source to INITIALIZATION_SOURCEFILES

Adding a NEW OPERATOR (internal, not user-accessible):

.. code-block:: text

   [ ] Same as above, but skip step 1 (no JSON descriptor)
   [ ] In CreateNullFormatBuilders(), construct an inline mho_json and call
       AddBuilderTypeWithFormat<MHO_NewOperatorBuilder>("name", fmt)

9. Worked Examples
==================

EXAMPLE A: Simple parameter (real scalar)
------------------------------------------

Control file: ``ref_freq 1400.0``

JSON (``ref_freq.json``):

.. code-block:: json

   {
       "name": "ref_freq",
       "statement_type": "parameter",
       "parameter_type": "config",
       "type": "real"
   }

Consumed as:

.. code-block:: cpp

   double refFreq = fParameterStore->GetAs<double>("/control/config/ref_freq");

EXAMPLE B: Per-station parameter
---------------------------------

Control file::

   if station E mount_type zenith_pointing
   if station G mount_type equitorial_mount

JSON (``mount_type.json``):

.. code-block:: json

   {
       "name": "mount_type",
       "statement_type": "parameter",
       "parameter_type": "station",
       "type": "string"
   }

Stored at: ``/control/station/Wf/mount_type``, ``/control/station/Gs/mount_type``

Consumed as:

.. code-block:: cpp

   std::string ref_id = fParameterStore->GetAs<std::string>("/ref_station/site_id");
   std::string mount = fParameterStore->GetAs<std::string>(
       "/control/station/" + ref_id + "/mount_type");

EXAMPLE C: Simple boolean operator (DC block)
----------------------------------------------

Control file: ``dc_block true``

JSON (``dc_block.json``):

.. code-block:: json

   {
       "name": "dc_block",
       "statement_type": "operator",
       "operator_category": "flagging",
       "type": "bool",
       "priority": 3.5
   }

Operator (``MHO_DCBlock : MHO_UnaryOperator<visibility_type>``):

- ``ExecuteInPlace()`` iterates over channels, zeroes DC spectral point

Builder (``MHO_DCBlockBuilder::Build()``):

- Reads ``fAttributes["value"]`` as bool
- If true, constructs MHO_DCBlock, sets args, registers with toolbox

EXAMPLE D: Compound operator (adhoc phase correction)
------------------------------------------------------

Control file: ``adhoc_phase sinewave``

JSON (``adhoc_phase.json``):

.. code-block:: json

   {
       "name": "adhoc_phase",
       "statement_type": "operator",
       "operator_category": "calibration",
       "type": "compound",
       "priority": 3.5,
       "parameters": {
           "algorithm_type": {"type": "string"}
       },
       "fields": ["algorithm_type"]
   }

Builder (``MHO_AdhocPhaseCorrectionBuilder::Build()``):

- Reads ``fAttributes["value"]["algorithm_type"]`` as string
- Switches on ``"sinewave"`` | ``"polynomial"`` | ``"file"`` to set mode
- Reads additional params from ParameterStore:
  ``/control/config/adhoc_tref``, ``/control/config/adhoc_amp``,
  ``/control/config/adhoc_period``, ``/control/config/adhoc_poly``,
  ``/control/station/<CODE>/adhoc_file``
- Constructs MHO_AdhocPhaseCorrection with mode and params

EXAMPLE E: Compound operator with multiple fields
---------------------------------------------------

Control file: ``pc_phases a,b 10.5 20.3``

JSON (``pc_phases.json``):

.. code-block:: json

   {
       "name": "pc_phases",
       "statement_type": "operator",
       "operator_category": "calibration",
       "type": "compound",
       "priority": 3.5,
       "parameters": {
           "channel_names": {"type": "string"},
           "pc_phases": {"type": "list_real"}
       },
       "fields": ["channel_names", "pc_phases"]
   }

Builder reads:

.. code-block:: cpp

   std::string chan_names =
       fAttributes["value"]["channel_names"].get<std::string>();
   std::vector<double> phases =
       fAttributes["value"]["pc_phases"].get<std::vector<double>>();
       
Note: Channel may be comma separated (e.g a,b,c ) or concatenated (e.g. abc). However, the legacy HOPS3 
concatenated syntax is only valid if all channel names are single-character only (limited to 64 channels).
There is no limit on channel number if multi-character channel names are used, but they must be comma separated.

EXAMPLE F: Internal operator (no control file access)
------------------------------------------------------

In ``CreateNullFormatBuilders()``:

.. code-block:: cpp

   mho_json fmt;
   fmt["name"] = "circ_field_rotation_corr";
   fmt["operator_category"] = "calibration";
   fmt["priority"] = 3.98;
   AddBuilderTypeWithFormat<MHO_CircularFieldRotationBuilder>(
       "circ_field_rotation_corr", fmt);

Builder reads all configuration from ParameterStore and ContainerStore, and operator triggers on its own internal logic.


Common Pitfalls
===============

1.  JSON name must match filename (without ``.json`` extension)
2.  For "station" parameters, the station code in the path is resolved from
    the "if station X" condition, not hardcoded
3.  Compound "fields" array order matters - it defines token consumption order
4.  List types in compound fields consume ALL remaining tokens, so place them
    last in the "fields" array
5.  Optional fields must be prefixed with ``"!"`` in the "fields" array, and must come last.
6.  Builders must always provide both constructors (production + test)
7.  Operators registered with ``replace_duplicates=true`` will be replaced per pass
    (useful for per-station corrections); ``false`` means the multiple operators with the same name can be present.
8.  Priority determines execution order within a category (lower = earlier)
9.  The format descriptor's "operator_category" must match a valid category
    string: ``"labeling"``, ``"selection"``, ``"flagging"``, ``"calibration"``, ``"prefit"``,
    ``"postfit"``, ``"finalize"``, or ``"default"``
10. Station-specific operators should use ``GetMatchingStationIdentifiers()`` to
    filter to the current baseline's stations (this is needed when encountering complex conditional statements)
11. Always check for ``nullptr`` after ``ContainerStore::GetObject()`` - missing data
    should be treated as a fatal error in ``Build()``
