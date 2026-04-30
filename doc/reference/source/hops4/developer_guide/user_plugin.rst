============================
HOPS Python Plugin User Guide
============================

This document describes how to add Python-based plugins to the HOPS fringe
fitter pipeline. There are three categories of Python plugins:

1. **Python Data Operators** -- inject Python functions into the calibration pipeline
2. **Custom Plot Functions** -- replace the default fringe plot with your own
3. **Python Control Files** -- write the entire control config as a Python script

Each category is covered below with the exact API, file placement, and
control file syntax required for each extension.

1. Where to Install Your Python Scripts
========================================

The HOPS embedded Python interpreter searches for python plugins in two directories:

(a) Default plugins directory:
    ``$HOPS_INSTALL/plugin_scripts/``
    (set by CMake variable ``PLUGINS_INSTALL_DIR``)

(b) User plugins directory (optional):
    ``$HOPS_USER_PLUGINS_DIR/``
    (set via the ``HOPS_USER_PLUGINS_DIR`` environment variable)

Both directories are automatically appended to Python's ``sys.path`` at startup.
You can either copy your python plugin into the PLUGINS_INSTALL_DIR, or to 
use a custom user directory, export before running fourfit:

.. code-block:: bash

   export HOPS_USER_PLUGINS_DIR=/home/user/my_hops_plugins

The Python scripts you write must be importable as Python modules. This means:

- A file ``my_plugin.py`` is importable as ``import my_plugin``
- A file ``my_pkg/module.py`` is importable as ``import my_pkg.module``
- If using packages, include ``__init__.py`` in the directory

The scripts do NOT need to be compiled. They are loaded by the embedded
interpreter at runtime. Similarly any dependencies (e.g. if you need pandas, etc.) 
your script requires must also be importable under whatever environment you are running fourfit.

2. Python Data Operators
=========================

A Python data operator injects a Python function into the fringe fitter's
operator pipeline. The function receives an ``MHO_PyFringeDataInterface`` object
giving it read/write access to visibilities, weights, parameters, and scan data.
While the python operator has full read/write access to these data containers, (they are 
exposed as numpy arrays), the C++ backend owns the memory, and any
resize/reshape/reallocate operations on these arrays will fail. 

2.1 Required API
-----------------

Your Python module must define a function with the following signature. The 
name can be anything, but it must be a free function accepting a fringe data 
interface object:

.. code-block:: python

   def my_operator(fringe_data_interface):
       ...

The function takes a single argument: a ``MHO_PyFringeDataInterface``.
It returns nothing (``None``). Any python exception raised will be caught by the
C++ side, logged, and subsequent execution will attempt to recover and continue.

2.2 Available Categories and Priorities
----------------------------------------

Python operators can be placed in any of these pipeline categories:

.. list-table::
   :header-rows: 1
   :widths: 20 20 16

   * - Category
     - Control keyword
     - Default priority
   * - labeling
     - python_labeling
     - 0.2 (runs first)
   * - flagging
     - python_flagging
     - 2.9
   * - calibration
     - python_calibration
     - 3.9
   * - prefit
     - python_prefit
     - 7.9
   * - postfit
     - python_postfit
     - 8.9
   * - finalize
     - python_finalize
     - 9.9 (runs after fitting, plot data available)

2.3 Control File Syntax
-------------------------

Add one line per python operator in your control file, example:

.. code-block:: text

   python_calibration   my_module   my_function   [optional_priority]

Where:

- ``python_calibration`` is the category keyword (``python_labeling``, ``python_flagging``,
  ``python_calibration``, ``python_prefit``, ``python_postfit``, ``python_finalize``)
- ``my_module`` is the Python module path (dot-separated, e.g. ``my_pkg.sub.module``)
- ``my_function`` is the function name inside the module
- ``[optional_priority]`` is an optional float to override the default execution order

Example control file lines:

.. code-block:: text

   python_calibration   fix_noema_jumps   fix_noema_jumps
   python_finalize      generate_pcphases  generate_pcphases   9.95

2.4 Complete Working Example
-----------------------------

File: ``$HOPS_USER_PLUGINS_DIR/my_calibration.py``

.. code-block:: python

   import numpy as np

   def zero_dc_point(fringe_data_interface):
       """Zero out DC point in each channel of visibility data."""

       # Get parameter store and container store
       pstore = fringe_data_interface.get_parameter_store()
       cstore = fringe_data_interface.get_container_store()

       # Read a parameter
       ref_station = pstore.get_by_path("/ref_station/mk4id")
       print(f"Reference station: {ref_station}")

       # Get the visibility object by UUID
       vis_uuid = pstore.get_by_path("/uuid/visibilities")
       vis_obj  = cstore.get_object(vis_uuid)

       if vis_obj is None:
           return

       # Get the data as a zero-copy numpy array (modifications affect C++ side)
       vis_arr = vis_obj.get_numpy_array()

       # Get axis information
       polprod_axis = vis_obj.get_axis(0)     # polarization products
       channel_axis = vis_obj.get_axis(1)     # channels
       time_axis    = vis_obj.get_axis(2)     # time (AP)
       freq_axis    = vis_obj.get_axis(3)     # spectral points

       # Apply a simple correction: zero out the first spectral point
       for pp in range(len(polprod_axis)):
           for ch in range(len(channel_axis)):
               for ti in range(len(time_axis)):
                   vis_arr[pp, ch, ti, 0] = 0.0 + 0.0j

       # You can also set parameters from python:
       pstore.set_by_path("/control/config/my_correction_applied", True)


3. Python Operator Toolbox Access
==================================

From Python 3 (calibration or postfit), you can retrieve and reconfigure
existing C++ calibration operators that were built by the control file
machinery. This is useful for dynamically adjusting operator parameters
based on runtime data.

3.1 Required Imports
---------------------

You MUST import ``pyMHO_Calibration`` for pybind11 to properly downcast ``MHO_Operator*``
pointers to their concrete derived types:

.. code-block:: python

   import pyMHO_Operators
   import pyMHO_Calibration

3.2 API
--------

.. code-block:: python

   toolbox = fringe_data_interface.get_operator_toolbox()

   # Get all operator names
   names = toolbox.get_operator_names()

   # Get all operators with a specific name (may return multiple)
   ops = toolbox.get_all_operators_by_name("operator_name")

   # Get operators by category
   cal_ops = toolbox.get_operators_by_category("calibration")

   # Get total operator count
   n = toolbox.get_n_operators()

3.3 Complete Working Example
-----------------------------

File: ``$HOPS_USER_PLUGINS_DIR/reconfigure_op.py``

.. code-block:: python

   import pyMHO_Operators
   import pyMHO_Calibration

   def adjust_pc_phase_offset(fringe_data_interface):
       """Find a pc_phase_offset_y operator and change its phase offset."""

       toolbox = fringe_data_interface.get_operator_toolbox()
       if toolbox is None:
           print("operator toolbox not available")
           return

       # Retrieve all operators named "pc_phase_offset_y"
       ops = toolbox.get_all_operators_by_name("pc_phase_offset_y")

       if not ops:
           print("operator not found in toolbox")
           return

       # Iterate and find the one for a specific station
       target_station = "E"
       for op in ops:
           stid = op.get_station_identifier()
           if stid == target_station:
               op.set_pc_phase_offset(140.0)
               # Re-initialize after reconfiguration
               op.initialize()
               print(f"Adjusted phase offset for station {stid}")


4. Custom Plot Functions
=========================

Replace the default fringe plot with your own Python matplotlib function.

4.1 Required API
-----------------

Your function must have this signature:

.. code-block:: python

   def my_plot_function(fringe_data_interface):
       plot_data = fringe_data_interface.get_plot_data()
       # plot_data is a dict with keys like:
       #   "PLOT_INFO", "DLYRATE", "MBD_AMP", "SBD_AMP", "XPSPEC-ABS",
       #   "XPSPEC-ARG", "SEG_AMP", "SEG_PHS", "NSeg", "NPlots", etc.
       import matplotlib.pyplot as plt
       plt.figure()
       plt.plot(...)
       plt.savefig("my_custom_plot.png", dpi=300)
       plt.close()

Note: You are not limited to just the data in the ``plot_data`` dictionary. You have full 
access to the visibilities, parameter store, and other data object from the fringe data interface,
so you can compute any special functions as needed in python. 
However, they will likely be slower than an equivalent implement in C++.

4.2 Control File Syntax
-------------------------

.. code-block:: text

   python_custom_plot   my_plot_module   my_plot_function

Where:

- ``my_plot_module`` is the Python module path (dot-separated)
- ``my_plot_function`` is the function to call

Example:

.. code-block:: text

   python_custom_plot   custom_fourfit_plot   make_fourfit_plot_wrapper

4.3 Plot Backend Selection
----------------------------

The plot backend is controlled by the ``plot_backend`` parameter:

.. code-block:: text

   plot_backend   matplotlib

When ``plot_backend`` is ``"matplotlib"``, the ``MHO_DefaultPythonPlotVisitor`` is
used. If ``python_custom_plot`` is set, it overrides the default module and
function. Otherwise, the default is:

- module: ``hops_visualization.fourfit_plot``
- function: ``make_fourfit_plot_wrapper``

4.4 Complete Working Example
------------------------------

File: ``$HOPS_USER_PLUGINS_DIR/my_simple_plot.py``

.. code-block:: python

   import matplotlib
   matplotlib.use("Agg")
   import matplotlib.pyplot as plt
   import numpy as np

   def simple_delay_rate_plot(fringe_data_interface):
       """Plot delay rate vs time and save to file."""

       plot_data = fringe_data_interface.get_plot_data()
       pstore    = fringe_data_interface.get_parameter_store()

       dlyrate = plot_data.get("DLYRATE", [])

       plt.figure(figsize=(10, 6))
       plt.plot(dlyrate, "b-", linewidth=1.0)
       plt.xlabel("Time integration")
       plt.ylabel("Delay rate (ns/s)")
       plt.title("Delay Rate Search")
       plt.grid(True)

       # Determine output file
       if pstore.is_present("/cmdline/disk_file"):
           out_file = pstore.get_by_path("/cmdline/disk_file")
       else:
           out_file = "simple_delay_rate.png"

       plt.savefig(out_file, dpi=300)
       plt.close()


5. Python Control Files
========================

Instead of writing a fourfit domain-specific language (DSL) control file, you can also 
write a Python script (``.py``) that defines the same configuration programmatically with some limitations.

5.1 Required API
-----------------

Your Python control file must define:

.. code-block:: python

   from hops_control import PassInfo, Config

   def configure(p: PassInfo, cfg: Config):
       ...

5.2 PassInfo (read-only)
--------------------------

``PassInfo`` describes the current fringe pass (baseline, source, etc.):

.. code-block:: python

   p.baseline       # full baseline string, e.g. "EG" or "Gs-Wf"
   p.ref_station    # Mk4 ID of reference station, e.g. "E"
   p.rem_station    # Mk4 ID of remote station, e.g. "G"
   p.source         # source name, e.g. "3C279"
   p.fgroup         # frequency group, e.g. "X"
   p.scan_name      # scan name/time string
   p.polprod        # polarization product, e.g. "RR"

Condition helper methods:

.. code-block:: python

   p.station("E")           # True if E is ref or remote
   p.baseline_match("GE")   # True if baseline matches "GE" (with wildcards)
   p.source_match("3C279")  # True if source matches
   p.fgroup_match("X")      # True if fgroup matches
   p.scan_before("100-1200") # True if current scan < given scan
   p.scan_after("100-1200")  # True if current scan > given scan
   p.scan_between("a", "b")  # True if a <= current scan <= b

5.3 Config (writer)
---------------------

``Config`` accumulates control statements. Every known control keyword defined in the JSON format 
specifiers is available as a method on this class, e.g:

.. code-block:: python

   cfg.ref_freq(215000.0)
   cfg.pc_mode("multitone")
   cfg.ion_smooth(True)
   cfg.weak_channel(0.1)
   cfg.samplers(["abcdefgh", "ijklmnop", "qrstuvwx", "yzABCDEF"])

Compound parameters (multiple arguments):

.. code-block:: python

   cfg.pc_phases_x("abcdefghijklmnop", [1.0, -2.0, 3.0, ...])
   cfg.pc_tonemask("cdejnprwBC", [2, 16, 16, 1, 16, 16, 16, 2, 16, 16])

Conditional blocks (mirrors DSL "if station X"):

.. code-block:: python

   with cfg.IF().station("E"):
       cfg.sampler_delay_x([-20, -20, -20, -20])

   with cfg.IF().baseline("GE"):
       cfg.ion_npts(11)

   with cfg.IF().source("3C279").AND().fgroup("X"):
       cfg.ref_freq(86000.0)

   with cfg.IF().station("E").OR().station("G"):
       cfg.weak_channel(0.05)

Convenience shortcuts for single-predicate conditionals:

.. code-block:: python

   with cfg.if_station("E"):
       ...
   with cfg.if_baseline("GE"):
       ...
   with cfg.if_source("3C279"):
       ...
   with cfg.if_fgroup("X"):
       ...

.. important::

   NESTED conditional blocks are NOT allowed! Combine predicates
   in a single chain if needed instead:

   .. code-block:: python

      with cfg.IF().station("G").AND().source("3C279"):  # OK
          ...
          
   This somewhat inflexible ``with`` construct is required in order to pass the conditional information across to the
   intermediate control statement representation that is consumed by the operator builders. 
   Full python ``if`` syntax is not supported for station/baseline/source specific parameters and operators.

5.4 Complete Working Example
------------------------------

File: ``my_config.py`` (used as control file directly (MUST have .py extension))

.. code-block:: python

   from hops_control import PassInfo, Config

   def configure(p: PassInfo, cfg: Config):
       # Global settings (always applied)
       cfg.ref_freq(215000.0)
       cfg.pc_mode("multitone")
       cfg.pc_period(1)
       cfg.ion_smooth(True)
       cfg.weak_channel(0.1)

       # Per-station settings
       with cfg.IF().station("E"):
           cfg.sampler_delay_x([-20, -20, -20, -20])
           cfg.sampler_delay_y([-20, -20, -20, -20])
           cfg.pc_delay_y(0.734)
           cfg.pc_phase_offset_y(138.2)

       with cfg.IF().station("G"):
           cfg.sampler_delay_x([-140, 180, 180, 180])
           cfg.sampler_delay_y([-140, 180, 180, 180])

       # Baseline-specific settings
       with cfg.IF().baseline("GE"):
           cfg.ion_npts(11)
           cfg.ion_win([-20.0, 20.0])

       # Source-specific frequency override
       with cfg.IF().source("3C279").AND().fgroup("X"):
           cfg.ref_freq(86000.0)

To use this as a control file, pass it to fourfit4 just like a ordinary control file (note .py extension!):

.. code-block:: bash

   fourfit4 -c my_config.py ...


6. Python API Reference
========================

6.1 MHO_PyFringeDataInterface (main entry point)
--------------------------------------------------

Methods:

``get_parameter_store()`` -> ``MHO_PyParameterStoreInterface``
    Get the current parameter store

``get_container_store()`` -> ``MHO_PyContainerStoreInterface``
    Get the data container store (visibilities, weights, etc.)

``get_scan_store()`` -> ``MHO_PyScanStoreInterface``
    Get the scan data store

``get_vex()`` -> ``dict``
    Get VEX/root metadata as a Python dictionary

``get_plot_data()`` -> ``dict``
    Get plot data dictionary (available in finalize category)

``get_operator_toolbox()`` -> ``MHO_OperatorToolbox``
    Get the operator toolbox to query/reconfigure C++ operators

6.2 MHO_PyParameterStoreInterface
-----------------------------------

``is_present(path: str)`` -> ``bool``
    Check if a parameter exists at the given path

``get_by_path(path: str)`` -> ``any``
    Get the value at the given path (int, float, str, list, or dict).
    Returns Python native types; JSON is auto-converted.

``set_by_path(path: str, value: any)``
    Set a parameter at the given path

``get_contents()`` -> ``dict``
    Get the entire parameter store as a dictionary

Parameter paths follow the convention:

- ``/config/<name>`` -- global config parameters
- ``/control/config/<name>`` -- control file config parameters
- ``/control/station/<CODE>/<name>`` -- per-station parameters
- ``/control/fit/<name>`` -- fit-time parameters
- ``/ref_station/<field>`` -- reference station info (mk4id, site_id, etc.)
- ``/rem_station/<field>`` -- remote station info
- ``/uuid/<object_name>`` -- UUID of named data objects
- ``/cmdline/<option>`` -- command-line options

6.3 MHO_PyContainerStoreInterface
-----------------------------------

``is_valid()`` -> ``bool``
    Check if the store is valid

``get_nobjects()`` -> ``int``
    Get the number of objects in the store

``is_object_present(uuid: str)`` -> ``bool``
    Check if an object with the given UUID exists

``get_object_id_list()`` -> ``list[dict]``
    Get a list of dicts with type_uuid, object_uuid, shortname

``get_object(uuid: str)`` -> ``MHO_PyTableContainer`` or ``dict`` or ``None``
    Get the data object by UUID. Returns:

    - ``MHO_PyTableContainer`` for visibility/weight/phasor types
    - ``dict`` for MHO_ObjectTags
    - ``None`` if not found

Useful UUID paths for run-time object identification/location:

- ``/uuid/visibilities`` -- visibility data
- ``/uuid/weights`` -- weight data
- ``/uuid/phasors`` -- fringe-fit phasor averages

6.4 MHO_PyTableContainer (for visibility/weight/phasor objects)
-----------------------------------------------------------------

``get_rank()`` -> ``int``
    Get the number of dimensions (4 for visibility_type)

``get_dimension(index: int)`` -> ``int``
    Get the size of dimension at index

``get_numpy_array()`` -> ``numpy.ndarray``
    Get the underlying data as a zero-copy numpy array.
    Modifications in Python are visible to C++.
    The array CANNOT be resized or reshaped.

``get_axis(index: int)`` -> ``list``
    Get the coordinate values for the axis at the given dimension index

``get_axis_metadata(index: int)`` -> ``dict``
    Get metadata for the axis at the given dimension index

``set_axis_metadata(index: int, metadata: dict)``
    Replace axis metadata

``set_axis_label(dim_index: int, coord_index: int, label_value: any)``
    Modify a single coordinate label on an axis

``get_metadata()`` -> ``dict``
    Get the table's metadata dictionary

``set_metadata(metadata: dict)``
    Replace the table's metadata

6.5 MHO_OperatorToolbox
-------------------------

``get_operator_names()`` -> ``list[str]``
    Get all operator names

``get_n_operators()`` -> ``int``
    Get total operator count

``get_all_operators_by_name(name: str)`` -> ``list[MHO_Operator]``
    Get all operators with the given name (sorted by priority)

``get_operators_by_category(category: str)`` -> ``list[MHO_Operator]``
    Get operators in a specific category

6.6 Common Data Types and Array Shapes
----------------------------------------

- ``visibility_type`` -- 4D complex array [polprod, channel, time, freq]
- ``weight_type`` -- 4D real array [polprod, channel, time, freq]
- ``phasor_type`` -- time/freq averaged complex phasors
- ``sbd_type`` -- single-band delay data
- ``station_coord_type`` -- station coordinates

For ``visibility_type``, the 4 axes are:

- Axis 0: polarization products (e.g. "RR", "LL", "RL", "LR", "XX", "XY")
- Axis 1: channel index (0, 1, 2, ...)
- Axis 2: time integration index (0, 1, 2, ...)
- Axis 3: spectral point within channel (frequency coordinate)


7. Pre-imported Modules
=========================

The following pybind11 modules are automatically imported at startup:

- ``pyMHO_Containers`` -- Container store, parameter store, table containers
- ``pyMHO_Operators`` -- Operator base class, operator toolbox
- ``pyMHO_Calibration`` -- Concrete calibration operator classes (for type downcasting)

You do NOT need to import these explicitly unless you need to access
specific types. However, if you use the operator toolbox to retrieve
C++ operators, you MUST import ``pyMHO_Calibration`` for proper type casting.

Additionally, the ``hops_control`` and ``hops_visualization`` Python packages
are installed with HOPS and can be imported:

.. code-block:: python

   from hops_control import PassInfo, Config    # for Python control files
   import hops_visualization                     # for plotting utilities


8. Debugging Tips
==================

1. **Import errors:** Make sure your script is in ``HOPS_DEFAULT_PLUGINS_DIR``
   or ``HOPS_USER_PLUGINS_DIR``. Check with:

   .. code-block:: bash

      echo $HOPS_DEFAULT_PLUGINS_DIR
      echo $HOPS_USER_PLUGINS_DIR

2. **Module not found:** Verify the module path in the control file uses
   dot syntax (e.g. ``my_pkg.my_module``), matching the directory structure.

3. **Function not found:** The function name in the control file must match
   exactly with the ``def`` name in the Python file.

4. **pybind11 cast errors:** If you see "Unable to convert call argument",
   you may need to import ``pyMHO_Calibration`` explicitly.

5. **Python exceptions in operator:** These are caught and logged. Check
   the fourfit log output for "python_bindings" tagged error messages.

6. **stdout buffering:** Print statements may not appear immediately. The
   embedded interpreter uses line-buffered stdout, but for real-time
   debugging consider writing to a file:

   .. code-block:: python

      with open("/tmp/my_debug.log", "a") as f:
          f.write(f"debug: {value}\n")

7. **numpy version:** The zero-copy numpy array sharing requires numpy.
   The C++ build links against whatever numpy is available at build time.
   Runtime numpy should be compatible.

8. **Plot data availability:** The ``get_plot_data()`` method only returns
   meaningful data in the "finalize" category. In earlier categories
   (calibration, prefit, postfit), plot data may be empty.


9. Quick Reference: Checklist
==============================

To add a new Python data operator:

- [ ] Write a Python function: ``def my_func(fringe_data_interface):``
- [ ] Place the ``.py`` file in ``$HOPS_USER_PLUGINS_DIR`` or default plugin dir
- [ ] Add to control file: ``python_calibration  my_module  my_func  [priority]``
- [ ] Verify the module is importable: ``python3 -c "import my_module"``

To add a custom plot function:

- [ ] Write a Python function: ``def my_plot(fringe_data_interface):``
- [ ] Use ``fringe_data_interface.get_plot_data()`` to get plot dictionary
- [ ] Place the ``.py`` file in the plugin directory
- [ ] Add to control file: ``python_custom_plot  my_plot_module  my_plot``
- [ ] Set ``plot_backend`` to ``"matplotlib"`` in the control file

To write a Python control file:

- [ ] Create a ``.py`` file with: ``from hops_control import PassInfo, Config``
- [ ] Define: ``def configure(p: PassInfo, cfg: Config):``
- [ ] Use ``cfg.<keyword>(value)`` for global settings
- [ ] Use ``with cfg.IF().station("X"):`` for conditional settings
- [ ] Pass the ``.py`` file to fourfit as the ``-c`` argument
