..  _Message:

Message
=======

Most of the applications in HOPS4 pass messages to ``stdout`` via the messaging library. This library controls the verbosity and formatting 
of program messages and has several configuration options that can be applied at either compile time or run time.

**Compile Time Options**

These following options can be toggled via ``ccmake`` terminal GUI. If they are changed, the software must be re-built in order to take effect.

.. list-table::
    :header-rows: 1

    * - Option
      - Description
      - Default
        
    * - HOPS_ENABLE_DEBUG_MSG
      - This option allows the user to entirely turn on/off additional debug messages by removing them from compilation.
      - ON

    * - HOPS_ENABLE_EXTRA_VERBOSE_MSG
      - When enabled, this option adds additional (file,line) origin information to all messages.
      - OFF

    * - HOPS_ENABLE_COLOR_MSG
      - Enables color messages on the terminal, message color is determined by type (debug, info, status, warning, error, fatal).
      - ON

**Run Time Options**

.. list-table::
   :header-rows: 1

   * - Option
     - Description

   * - ``-M`` TEXT ..., ``--message-categories`` TEXT ...
     - Limit the allowed message categories to only those which the user specifies. Available categories:
       
       - ``main``, messages from the top-level program
       - ``calibration``, messages from the calibration library
       - ``containers``, messages from the container library
       - ``control``, messages from the control file parsing library
       - ``fringe``, messages from the fringe-fitting library
       - ``file``, messages from utilities library pertaining to file operations
       - ``initialization``, messages from the (operator) initialization library
       - ``mk4interface``, messages from the mark4 file interface library
       - ``utilities``, messages from the utilities library
       - ``vex``, message from the vex parsing library
       - ``plot``, messages from fringe plot generation
       - ``python_bindings``, messages from the python interface library

          *Message categories from optional plugins/libraries:*

       - ``mpi_interface``, (optional) messages from the MPI interface library
       - ``difx_interface``, (optional) messages from the DiFX interface library (difx2hops)
       - ``hdf5_interface``, (optional) messages from the HDF5 export library
       - ``opencl``, (optional) messages from the OpenCL plugin
       - ``cuda``, (optional) messages from the CUDA plugin library

       If not specified, all categories are allowed. 
       Multiple categories can be specified as a comma separate list (e.g. ``-M main,vex,fringe``).
   * - ``-m`` INT, ``--message-level`` INT
     - Message verbosity level, range: ``-2`` (debug) to ``5`` (silent)

**Usage**

Application and library code emits messages via a family of severity macros, each
tied to a ``MHO_MessageLevel`` enumerator, taking a message category key and content
terminated by the ``eom`` stream manipulator:

.. code-block:: cpp

    msg_status("fringe", "starting fringe fit for scan " << scan_name << eom);

The available severity macros, from most to least severe, are:

.. list-table::
    :header-rows: 1

    * - Macro
      - Level
    * - ``msg_fatal(key, content)``
      - ``eFatalErrorLevel`` -- fatal error, program termination imminent
    * - ``msg_error(key, content)``
      - ``eErrorLevel`` -- non-fatal error which may lead to unexpected behavior
    * - ``msg_warn(key, content)``
      - ``eWarningLevel`` -- unexpected state which may lead to errors
    * - ``msg_status(key, content)``
      - ``eStatusLevel`` -- information about current execution status
    * - ``msg_info(key, content)``
      - ``eInfoLevel`` -- extra information about program configuration/state
    * - ``msg_debug(key, content)``
      - ``eDebugLevel`` -- debug information of interest primarily to developers;
        compiled out entirely unless ``HOPS_ENABLE_DEBUG_MSG`` is set

See the implementation details of the class :hops:`hops::MHO_Message` for further information on messaging configuration.
