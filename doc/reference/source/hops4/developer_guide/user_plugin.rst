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
    ``${CMAKE_INSTALL_PREFIX}/plugin_scripts/``

    This path is baked into the fourfit binary at compile time via the CMake variable
    ``PLUGINS_INSTALL_DIR``. It is **not** an environment variable you can override at
    runtime. The actual path is printed to the fourfit log at startup (tagged
    ``[main]``), which is the easiest way to confirm it.

(b) User plugins directory (optional):
    ``$HOPS_USER_PLUGINS_DIR/``
    (set via the ``HOPS_USER_PLUGINS_DIR`` environment variable)

Both directories are appended to Python's ``sys.path`` at startup.
To use a custom user directory, export it before running fourfit:

.. code-block:: bash

   export HOPS_USER_PLUGINS_DIR=/home/user/my_hops_plugins

The Python scripts you write must be importable as Python modules. This means:

- A file ``my_plugin.py`` is importable as ``import my_plugin``
- A file ``my_pkg/module.py`` is importable as ``import my_pkg.module``
- If using packages, include ``__init__.py`` in the directory

The scripts do NOT need to be compiled. They are loaded by the embedded
interpreter at runtime. Any third-party dependencies your script requires
(e.g. ``pandas``) must also be importable in the same Python environment
that fourfit was built against.

2. Python Data Operators
=========================

A Python data operator injects a Python function into the fringe fitter's
operator pipeline. The function receives an ``MHO_PyFringeDataInterface`` object
giving it read/write access to visibilities, weights, parameters, and scan data.
While the Python operator has full read/write access to these data containers
(exposed as numpy arrays), the C++ backend owns the memory. Any attempt to
resize, reshape, or reallocate these arrays from Python will fail.

2.1 Required API
-----------------

Your Python module must define a function with the following signature. The
name can be anything, but it must be a free function accepting a fringe data
interface object:

.. code-block:: python

   def my_operator(fringe_data_interface):
       ...

The function takes a single argument: a ``MHO_PyFringeDataInterface``.
It returns nothing (``None``). Any Python exception raised will be caught by the
C++ side, logged under the ``python_bindings`` tag, and execution will attempt
to continue with the next pipeline stage.

.. note::

   ``sys.exit()`` / ``SystemExit`` is an exception to this rule: if your plugin
   calls ``sys.exit()``, the embedded interpreter propagates it and fourfit exits
   immediately rather than logging and continuing.

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

From any Python operator (any category), you can retrieve and reconfigure
existing C++ calibration operators that were built by the control file
machinery. This is useful for dynamically adjusting operator parameters
based on runtime data.

3.1 Imports
------------

``pyMHO_Containers``, ``pyMHO_Operators``, and ``pyMHO_Calibration`` are all
imported automatically by the embedded interpreter before any user plugin is
called (see :ref:`sec-pre-imported-modules`). You do **not** need to import
them in your plugin scripts for the toolbox and type downcasting to work.

If you want explicit imports for IDE autocompletion or clarity:

.. code-block:: python

   import pyMHO_Operators
   import pyMHO_Calibration

3.2 API
--------

.. code-block:: python

   toolbox = fringe_data_interface.get_operator_toolbox()
   if toolbox is None:
       return  # toolbox not available in this context

   # Get all operator names (one entry per operator, in priority order)
   names = toolbox.get_operator_names()

   # Get all operators with a specific name (returns list sorted by priority)
   ops = toolbox.get_all_operators_by_name("operator_name")

   # Get operators in a specific category (sorted by priority)
   cal_ops = toolbox.get_operators_by_category("calibration")

   # Get total operator count
   n = toolbox.get_n_operators()

All list-returning methods return operators sorted by ascending priority.
The returned operator objects are references into C++-owned memory; do not
store them past the end of your plugin function.

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
       import matplotlib.pyplot as plt
       plt.figure()
       plt.plot(...)
       plt.savefig("my_custom_plot.png", dpi=300)
       plt.close()

The ``plot_data`` dictionary contains the following top-level keys (a subset
may be absent if the corresponding data was not computed):

- ``PLOT_INFO`` -- per-channel table (dict of lists; see below)
- ``DLYRATE``, ``DLYRATE_XAXIS`` -- delay-rate search amplitude and axis values
- ``MBD_AMP``, ``MBD_AMP_XAXIS`` -- multiband delay search amplitude and axis
- ``SBD_AMP``, ``SBD_AMP_XAXIS`` -- singleband delay amplitude and axis
- ``XPSPEC-ABS``, ``XPSPEC-ARG``, ``XPSPEC_XAXIS`` -- cross-power spectrum (amplitude, phase, freq axis)
- ``SEG_AMP``, ``SEG_PHS`` -- segment amplitudes and phases
- ``SEG_FRAC_USB``, ``SEG_FRAC_LSB`` -- USB/LSB validity fractions per segment
- ``NSeg``, ``NPlots`` -- number of time segments and channel plots
- ``ChannelsPlotted`` -- list of channel labels included in the plot (optional)
- ``Quality``, ``SNR``, ``Amp``, ``ResPhase``, ``PFD``, ``IntgTime`` -- fringe quality metrics
- ``ResidSbd(us)``, ``ResidMbd(us)`` -- residual single- and multiband delays
- ``FringeRate(Hz)``, ``IonTEC(TEC)`` -- fringe rate and ionospheric TEC
- ``RefFreq(MHz)``, ``AP(sec)`` -- reference frequency and accumulation period
- ``ExperName``, ``ExperNum``, ``YearDOY``, ``Start``, ``Stop`` -- observation metadata
- ``FRT``, ``CorrTime``, ``FFTime``, ``BuildTime`` -- processing timestamps
- ``RA``, ``Dec`` -- source coordinates
- ``RootScanBaseline``, ``CorrVers``, ``PolStr`` -- header identification fields
- ``extra`` -- dict of optional supplementary fields (see below)

Selected ``extra`` sub-keys:

- ``pol_product`` -- polarization product string (e.g. ``"RR"``, ``"I"`` for pseudo-Stokes)
- ``ref_station``, ``rem_station`` -- dicts with az, el, pa, u, v per station
- ``u``, ``v`` -- UV baseline coordinates
- ``sb_win``, ``mb_win``, ``dr_win``, ``ion_win`` -- search window limits ``[min, max]``
- ``ref_mtpc_phase_segs``, ``rem_mtpc_phase_segs`` -- multitone PCAL phase segments per channel
- ``dtec_array``, ``dtec_amp_array`` -- ionospheric dispersion data (when ionosphere fitting is enabled)

Note: you are not limited to the data in ``plot_data``. The full fringe data
interface gives you access to visibilities, weights, and the parameter store,
so you can compute any derived quantities in Python. Those computations will
generally be slower than equivalent C++ implementations.

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

The plot backend is controlled by the ``plot_backend`` parameter in the
control file:

.. code-block:: text

   plot_backend   matplotlib

When ``plot_backend`` is ``"matplotlib"``, the ``MHO_DefaultPythonPlotVisitor``
is used to call a Python matplotlib-based plot function.

If ``python_custom_plot`` is also set, it overrides the module and function
that the visitor calls. If ``python_custom_plot`` is not set, the defaults are:

- module: ``hops_visualization.fourfit_plot``
- function: ``make_fourfit_plot_wrapper``

If ``plot_backend`` is not set in the control file and gnuplot support was
compiled in, gnuplot is used instead. When gnuplot support is not compiled
in, matplotlib is the fallback.

.. note::

   ``python_custom_plot`` only takes effect when ``plot_backend matplotlib``
   is also set (or when matplotlib is selected as the fallback). Setting
   ``python_custom_plot`` alone has no effect if gnuplot is active.

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

Instead of writing a fourfit domain-specific language (DSL) control file, you
can write a Python script (``.py``) that defines the same configuration
programmatically. This approach supports conditional logic but has some
structural constraints described below.

5.1 Required API
-----------------

Your Python control file must define:

.. code-block:: python

   from hops_control import PassInfo, Config

   def configure(p: PassInfo, cfg: Config):
       ...

5.2 PassInfo (read-only)
--------------------------

``PassInfo`` is an immutable description of the fringe pass (baseline, source, etc.)
constructed by the C++ runtime before ``configure()`` is called.

Read-only properties:

.. code-block:: python

   p.baseline       # full baseline string, e.g. "EG" or "Gs-Wf"
   p.ref_station    # single-char Mk4 ID of reference station, e.g. "E"
   p.rem_station    # single-char Mk4 ID of remote station, e.g. "G"
   p.source         # source name, e.g. "3C279"
   p.fgroup         # frequency group character, e.g. "X"
   p.scan_name      # scan name/time string (used for ordering comparisons)
   p.polprod        # polarization product, e.g. "RR"

Condition helper methods (use these inside ``cfg.IF()`` chains, or for plain
Python conditionals -- both are valid):

.. code-block:: python

   p.station("E")            # True if "E" is ref or remote Mk4 ID;
                             # pass canonical name (e.g. "Gs") to match by long code;
                             # "?" always returns True (wildcard)
   p.baseline_match("GE")   # True if baseline matches; supports "?" wildcard per station
   p.source_match("3C279")  # True if source matches; "?" is wildcard
   p.fgroup_match("X")      # True if fgroup matches; "?" is wildcard
   p.scan_before("100-1200") # True if current scan name < argument (lexicographic)
   p.scan_after("100-1200")  # True if current scan name > argument (lexicographic)
   p.scan_between("a", "b")  # True if a <= scan_name <= b (inclusive, lexicographic)

5.3 Config (writer)
---------------------

``Config`` accumulates control statements and serializes them into the
intermediate representation consumed by the operator builders. Every control
keyword defined in the JSON format specifiers is available as a method on
this class. To see all available keywords at runtime:

.. code-block:: python

   print(cfg.available_keywords())

Example usage:

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

   **Nested conditional blocks are not allowed.** Attempting to nest ``with``
   blocks raises a ``RuntimeError`` at runtime. Combine all predicates in a
   single chain instead:

   .. code-block:: python

      # OK: combine with .AND() / .OR() in one chain
      with cfg.IF().station("G").AND().source("3C279"):
          ...

      # NOT OK: raises RuntimeError
      with cfg.IF().station("G"):
          with cfg.IF().source("3C279"):   # <-- error
              ...

   The ``with``-block construct is required because the conditional
   information must be passed to an intermediate control-statement
   representation before the operator builders consume it. Standard Python
   ``if`` statements (e.g. ``if p.station("G"):``) are fine for non-conditional
   global statements but cannot produce the station/baseline/source-conditional
   blocks that the operator pipeline needs.

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

To use this as a control file, pass it to fourfit4 just like an ordinary control file (note the ``.py`` extension):

.. code-block:: bash

   fourfit4 -c my_config.py ...


.. _sec-python-api:

6. Python API Reference
========================

.. _sec-fringe-data-interface:

6.1 MHO_PyFringeDataInterface (main entry point)
--------------------------------------------------

Methods:

``get_parameter_store()`` -> ``MHO_PyParameterStoreInterface``
    Get the current parameter store (returns a reference; not a copy).

``get_container_store()`` -> ``MHO_PyContainerStoreInterface``
    Get the data container store (visibilities, weights, etc.; returns a
    reference).

``get_scan_store()`` -> ``MHO_PyScanStoreInterface``
    Get the scan data store (returns a reference).

``get_vex()`` -> ``dict``
    Get VEX/root metadata as a Python dictionary (returns a copy).

``get_plot_data()`` -> ``dict``
    Get the plot data dictionary (returns a copy). Only populated after the
    fringe fit completes; in all pipeline categories before ``finalize``, this
    dict will be empty.

``get_operator_toolbox()`` -> ``MHO_OperatorToolbox`` or ``None``
    Get the operator toolbox to query/reconfigure C++ operators. Returns
    ``None`` when the toolbox has not been wired up (e.g. in stand-alone
    plotting contexts); always check for ``None`` before use.

6.2 MHO_PyParameterStoreInterface
-----------------------------------

``is_present(path: str)`` -> ``bool``
    Return ``True`` if a parameter exists at the given path.

``get_by_path(path: str)`` -> ``any``
    Get the value at the given path. Returns Python native types (int, float,
    str, list, or dict); JSON objects are auto-converted. If the path does not
    exist, prints an error to the console and returns an empty/``None`` object.
    Use ``is_present()`` first if the path may be absent.

``set_by_path(path: str, value: any)``
    Set a parameter at the given path. Value must be JSON-serialisable.

``get_contents()`` -> ``dict``
    Get the entire parameter store as a nested dictionary.

Parameter paths follow the convention:

- ``/config/<name>`` -- global config parameters
- ``/control/config/<name>`` -- control file config parameters
- ``/control/station/<CODE>/<name>`` -- per-station parameters
- ``/control/fit/<name>`` -- fit-time parameters
- ``/ref_station/<field>`` -- reference station info (mk4id, site_id, etc.)
- ``/rem_station/<field>`` -- remote station info
- ``/uuid/<object_name>`` -- UUID of named data objects
- ``/cmdline/<option>`` -- command-line options
- ``/status/<flag>`` -- runtime status flags (e.g. ``/status/skipped``)

6.3 MHO_PyContainerStoreInterface
-----------------------------------

``is_valid()`` -> ``bool``
    Return ``True`` if the store is valid.

``get_nobjects()`` -> ``int``
    Return the number of objects in the store.

``is_object_present(uuid: str)`` -> ``bool``
    Return ``True`` if an object with the given UUID exists.

``get_object_id_list()`` -> ``list[dict]``
    Return a list of dicts, each with keys ``"type_uuid"``,
    ``"object_uuid"``, and ``"shortname"``.

``get_object(uuid: str)`` -> ``MHO_PyTableContainer`` or ``dict`` or ``None``
    Retrieve the data object by UUID. The return type depends on the
    underlying data type:

    - ``MHO_PyTableContainer`` for array types: ``visibility_type``,
      ``weight_type``, ``phasor_type``, ``station_coord_type``,
      ``multitone_pcal_type``, ``visibility_store_type``,
      ``weight_store_type``
    - ``dict`` for ``MHO_ObjectTags`` (tag/metadata objects)
    - ``None`` if the UUID is not found

Useful UUID paths for runtime object lookup:

- ``/uuid/visibilities`` -- visibility data
- ``/uuid/weights`` -- weight data
- ``/uuid/phasors`` -- fringe-fit phasor averages

6.4 MHO_PyTableContainer (for visibility/weight/phasor objects)
-----------------------------------------------------------------

``get_rank()`` -> ``int``
    Return the number of dimensions (4 for ``visibility_type``).

``get_dimension(index: int)`` -> ``int``
    Return the size of dimension ``index``.

``get_classname()`` -> ``str``
    Return the C++ type name of the underlying container (e.g.
    ``"visibility_type"``).

``get_numpy_array()`` -> ``numpy.ndarray``
    Return the underlying data as a **zero-copy** numpy array backed by
    C++ memory. Modifications in Python are immediately visible on the C++
    side. The array **cannot** be resized, reshaped, or reallocated.

``get_axis(index: int)`` -> ``list``
    Return the coordinate labels for the axis at dimension ``index`` as a
    new Python list (a copy, not a view into C++ memory).

``get_axis_metadata(index: int)`` -> ``dict``
    Return metadata for the axis at dimension ``index`` as a dict. The dict
    includes an ``"index_labels"`` sub-dict for per-coordinate metadata
    (e.g. per-channel information for axis 1 of a visibility array).

``set_axis_metadata(index: int, metadata: dict)``
    Replace the entire axis metadata dict. Derive the new value from
    ``get_axis_metadata()`` first to avoid discarding existing fields.

``set_axis_label(dim_index: int, coord_index: int, label_value: any)``
    Modify a single coordinate label on an axis. The type of ``label_value``
    must match the axis element type or a cast exception will be raised.

``get_metadata()`` -> ``dict``
    Return the table-level metadata dictionary.

``set_metadata(metadata: dict)``
    Replace the table-level metadata dictionary.

6.5 MHO_OperatorToolbox
-------------------------

``get_operator_names()`` -> ``list[str]``
    Return the name of every operator in the toolbox, in priority order
    (one entry per operator, including duplicates with the same name).

``get_n_operators()`` -> ``int``
    Return the total operator count.

``get_all_operators_by_name(name: str)`` -> ``list[MHO_Operator]``
    Return all operators whose name equals ``name``, sorted by ascending
    priority. Returns an empty list if none are found.

``get_operators_by_category(category: str)`` -> ``list[MHO_Operator]``
    Return all operators in the given category (e.g. ``"calibration"``,
    ``"flagging"``), sorted by ascending priority.

All returned operator objects are references into C++-owned memory.
Do **not** store them beyond the scope of your plugin function.
``pyMHO_Calibration`` is pre-imported at startup, so pybind11 can always
downcast the base ``MHO_Operator*`` pointers to their concrete derived types
without any explicit import in your script.

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


.. _sec-pre-imported-modules:

7. Pre-imported Modules
=========================

The following pybind11 extension modules are imported automatically by the
embedded interpreter before any user plugin is called:

- ``pyMHO_Containers`` -- Container store, parameter store, table containers
- ``pyMHO_Operators`` -- Operator base class, operator toolbox, and the
  ``get_operator_toolbox()`` method on ``MHO_PyFringeDataInterface``
- ``pyMHO_Calibration`` -- Concrete calibration operator classes (required
  for pybind11 to downcast ``MHO_Operator*`` pointers to derived types)

Because all three are pre-loaded, you do **not** need to import them in
your plugin scripts for downcasting or toolbox access to work. Adding
explicit ``import`` statements in your scripts is harmless and can be
useful for IDE autocompletion.

The ``hops_control`` and ``hops_visualization`` Python packages are
installed alongside HOPS and can be imported normally:

.. code-block:: python

   from hops_control import PassInfo, Config    # for Python control files
   import hops_visualization                     # for plotting utilities


8. Debugging Tips
==================

1. **Import errors:** Make sure your script is in ``$HOPS_USER_PLUGINS_DIR``
   or the default plugins directory. The default directory path is baked into
   the binary at compile time (not an environment variable); fourfit prints it
   to the log at startup tagged ``[main]``. You can also confirm your user
   directory:

   .. code-block:: bash

      echo $HOPS_USER_PLUGINS_DIR

2. **Module not found:** Verify the module path in the control file uses
   dot syntax (e.g. ``my_pkg.my_module``), matching the directory structure
   under the plugins directory.

3. **Function not found:** The function name in the control file must match
   exactly the ``def`` name in the Python file (case-sensitive).

4. **pybind11 cast errors:** If you see ``"Unable to convert call argument"``
   or ``"terminate called after throwing an instance of 'pybind11::cast_error'"``,
   make sure ``pyMHO_Calibration`` has been imported. It is pre-imported
   automatically, so this error usually indicates the embedded interpreter was
   not initialised before your script ran (which should not happen under normal
   fourfit operation).

5. **Python exceptions in operator:** Exceptions are caught by the C++ side,
   logged, and execution continues. Check the fourfit log for messages tagged
   ``python_bindings``. Note that ``sys.exit()`` / ``SystemExit`` is **not**
   caught -- it terminates fourfit immediately.

6. **stdout buffering:** The embedded interpreter reconfigures ``sys.stdout``
   to line-buffered mode, so ``print()`` output should appear promptly. If you
   still see delays, write directly to a file:

   .. code-block:: python

      with open("/tmp/my_debug.log", "a") as f:
          f.write(f"debug: {value}\n")

7. **numpy version:** The zero-copy numpy array relies on numpy being available
   at both build time and runtime. The C++ build links against the numpy
   present at compile time; the runtime numpy should be compatible (same major
   version).

8. **Plot data availability:** ``get_plot_data()`` only returns populated data
   in the ``finalize`` pipeline category. In earlier categories (``calibration``,
   ``prefit``, ``postfit``), the plot data dict will be empty.


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
