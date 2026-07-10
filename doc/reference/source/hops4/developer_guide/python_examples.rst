Python Examples
~~~~~~~~~~~~~~~

The Python examples demonstrate a couple practical applications of the Python bindings
for solving real-world VLBI data processing problems. These examples show how
to use the Python API for custom calibration, data correction, and analysis tasks.
Python plugins can be used to correct known instrumental problems and
preprocess data before fringe fitting. They can also support automated
generation and refinement of calibration corrections, assessment of phase
stability across channels, and development of other custom calibration
algorithms for specialized or multi-pass processing workflows.

Minimal Plugin (example1.py)
-----------------------------

=============================================== ====================================================================
File                                            example1.py
Category                                        Python Examples
Function                                        dummy(fringe_data_interface)
Primary Functionality                           Minimal "hello world" plugin
Key Features                                    | Parameter store lookup by path
                                                | Reference/remote station identification
=============================================== ====================================================================

The simplest possible plugin. It only demonstrates how to access the parameters store by looking up the reference and remote station
Mark4 IDs and prints them. You can use this as a basic starting point when writing a new plugin.

**Implementation Details:**

.. code-block:: python

   def dummy(fringe_data_interface):
       param_interface_obj = fringe_data_interface.get_parameter_store()

       ref_station_id = param_interface_obj.get_by_path("/ref_station/mk4id")
       rem_station_id = param_interface_obj.get_by_path("/rem_station/mk4id")

       print("reference station", ref_station_id, ", remote station: ", rem_station_id)


NOEMA Phase Jump Correction (example2.py)
------------------------------------------

=============================================== ====================================================================
File                                            example2.py
Category                                        Python Examples
Function                                        fix_noema_jumps(fringe_data_interface)
Primary Functionality                           Corrects known phase jumps in NOEMA telescope data
Key Features                                    | Channel-specific phase corrections
                                                | Polarization-dependent adjustments
                                                | Frequency-based jump detection
                                                | Direct manipulation of visibility arrays
=============================================== ====================================================================

The NOEMA phase jump correction example demonstrates how to use the Python API
to correct known instrumental phase jumps in visibility data.

**Implementation Details:**

.. code-block:: python

   def fix_noema_jumps(fringe_data_interface):
       cstore_interface_obj = fringe_data_interface.get_container_store()
       param_interface_obj = fringe_data_interface.get_parameter_store()

       # grab the UUID of the visibility object
       vis_uuid = param_interface_obj.get_by_path("/uuid/visibilities")
       visib_obj = cstore_interface_obj.get_object(vis_uuid)

       if visib_obj is None:
           return  # bail out

       # figure out if NOEMA is reference or remote station
       stidx = 0
       ref_id = param_interface_obj.get_by_path("/ref_station/site_id")
       rem_id = param_interface_obj.get_by_path("/rem_station/site_id")
       if ref_id == "Nn":
           stidx = 0
       if rem_id == "Nn":
           stidx = 1

       # grab the underlying visibility 4-d array and the axis information we care about
       vis_arr = visib_obj.get_numpy_array()
       axis0 = visib_obj.get_axis(0)               # polprod axis
       axis1 = visib_obj.get_axis(1)                # channel axis
       axis3 = visib_obj.get_axis(3)                # spectral point axis (sub-channel)
       chan_meta_data = visib_obj.get_axis_metadata(1)  # channel axis meta data object
       channel_info = chan_meta_data["index_labels"]    # channel bin label dict

       #Note: these dictionaries are truncated for this example, see the source code for entirety
       jumps_l = {
           "a": [25.0, -1.0],
       }

       jumps_r = {
           "a": [25.0, -1.0],
       }

       # stash both pols in one object
       jumps = dict()
       jumps["R"] = jumps_r
       jumps["L"] = jumps_l

       # apply the corrections, looping over pol-product, channel, and spectral point
       nspectral = visib_obj.get_dimension(3)
       for pp in range(0, len(axis0)):
           polprod = axis0[pp]
           pol = polprod[stidx:stidx + 1]
           for ch in range(0, len(axis1)):
               chan_key = str(ch)  # meta data keys must be strings, not integers
               fourfit_chan_label = channel_info[chan_key]["channel_label"]
               if pol in jumps and fourfit_chan_label in jumps[pol]:
                   jump_freq, jump_phasor = jumps[pol][fourfit_chan_label]
                   for sp in range(0, nspectral):
                       freq = axis3[sp]
                       if freq > jump_freq:
                           vis_arr[pp, ch, :, sp] *= jump_phasor


Phase Calibration Generation (example3.py)
-------------------------------------------

=============================================== ====================================================================
File                                            example3.py
Category                                        Python Examples
Function                                        generate_pcphases(fringe_data_interface)
Primary Functionality                           Generates phase calibration corrections from fringe fit results
Key Features                                    | Plot data analysis for phase residuals
                                                | Channel-based phase residual computation
                                                | Circular mean phase calculation
                                                | Fourfit-compatible output format generation
=============================================== ====================================================================

The phase calibration generation example shows how to derive manual phase calibration
corrections from fringe fitting results and generate output compatible with
the fourfit control file format.

**Implementation Details:**

.. code-block:: python

   import numpy as np
   import scipy.stats
   from vpal import utility

   def generate_pcphases(fringe_data_interface):

       # keep in mind that 'plot_data' is only available in the 'finalize' step
       # so this function must be called as a 'python_finalize' operator
       plot_data = fringe_data_interface.get_plot_data()
       param_interface_obj = fringe_data_interface.get_parameter_store()

       # get the remote station mk4id (this function always generates pc_phases
       # as if they were for the remote_station)
       rem_station_id = param_interface_obj.get_by_path("/rem_station/mk4id")

       # get channel label and phase
       ch_labels = plot_data["PLOT_INFO"]["#Ch"]
       ch_phase = plot_data["PLOT_INFO"]["Phase"]

       nchan = len(ch_labels) - 1
       phase_residuals = dict()
       for i in list(range(0, nchan)):
           chan_id = ch_labels[i]
           phase_residuals[chan_id] = ch_phase[i]  # in degrees

       # now compute the phase corrections
       phase_corrections = phase_residuals.copy()
       phase_list_proxy = []
       channel_list = []
       for ch, ch_phase in list(phase_corrections.items()):
           channel_list.append(ch)
           phase_list_proxy.append(-1.0 * ch_phase)  # invert to get corrections from residuals

       # compute the circular mean phase, then subtract it off and limit to [-180, 180)
       mean_phase = scipy.stats.circmean(np.asarray(phase_list_proxy), high=180.0, low=-180.0)
       phase_list_proxy = [utility.limit_periodic_quantity_to_range((x - mean_phase), -180.0, 180.0)
                            for x in phase_list_proxy]

       # assign the corrections
       for i in list(range(0, len(phase_list_proxy))):
           phase_corrections[channel_list[i]] = phase_list_proxy[i]

       chan_names = ""
       phase_list_str = ""
       for elem in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789":
           if elem in channel_list:
               chan_names += elem
               phase_list_str += str(round(phase_corrections[elem], 2)) + " "

       print("if station", rem_station_id, " pc_phases ", chan_names, phase_list_str)

This example depends on ``scipy.stats.circmean`` for the circular mean phase
calculation and on ``vpal.utility.limit_periodic_quantity_to_range`` (from the
``vpal`` module shipped with HOPS) to wrap phases into ``[-180, 180)``.

**Output Format:**
The generated output is a single ``if station ... pc_phases ...`` line,
matching the control-file conditional syntax, and can be pasted directly into
a fourfit control file, e.g.:

.. code-block:: text

   if station G  pc_phases  abcd  1.2 -3.4 5.6 -7.8

Operator Toolbox Reconfiguration (example4.py)
------------------------------------------------

=============================================== ====================================================================
File                                            example4.py
Category                                        Python Examples
Function                                        set_pc_phase_offset_y(fringe_data_interface)
Primary Functionality                           Retrieves and reconfigures a calibration operator from the toolbox
Key Features                                    | Operator toolbox inspection
                                                | Lookup of operators by name and by station
                                                | Runtime reconfiguration of an operator's parameters
                                                | Re-initialization after reconfiguration
=============================================== ====================================================================

This example demonstrates how to retrieve an already-constructed calibration
operator from the operator toolbox and override its configuration at runtime.
It retrieves the ``pc_phase_offset_y`` operator (an ``MHO_ManualPolPhaseCorrection``
instance created by the builder when the control file contains a
``pc_phase_offset_y`` statement), finds the copy that matches a specific
station, and overrides its phase offset.

**Implementation Details:**

.. code-block:: python

   import pyMHO_Containers
   import pyMHO_Operators
   import pyMHO_Calibration  # must import so pybind11 can downcast MHO_Operator* to the correct type

   def set_pc_phase_offset_y(fringe_data_interface):
       toolbox = fringe_data_interface.get_operator_toolbox()

       if toolbox is None:
           print("example4: operator toolbox not available")
           return

       print("example4: operators in toolbox:")
       for name in toolbox.get_operator_names():
           print(f"  {name}")

       # Because pyMHO_Calibration is imported, pybind11 downcasts the returned
       # MHO_Operator* to MHO_ManualPolPhaseCorrection. Multiple copies may have
       # been created by the control file machinery, so inspect each one for the
       # station we're interested in.
       station_mk4_id = "E"   # westford
       station_id = "Wf"      # westford
       ops = toolbox.get_all_operators_by_name("pc_phase_offset_y")

       if len(ops) == 0:
           print("example4: 'pc_phase_offset_y' operator not found in toolbox "
                 "(check that the control file contains a pc_phase_offset_y statement)")
           return

       ph_off = 140.0
       for op in ops:
           stid = op.get_station_identifier()
           if stid == station_mk4_id or stid == station_id:
               print(f"example4: found operator '{op.get_name()}', for station: '{stid}', setting phase offset to '{ph_off}'")
               op.set_pc_phase_offset(ph_off)
               # Re-initialize so the new offset is picked up before Execute() is called.
               ok = op.initialize()
               if not ok:
                   print("example4: warning - initialize() returned False after reconfiguration")

Note that after changing an operator's configuration, ``initialize()`` must be
called so the operator re-caches any pre-computed values before ``Execute()``
runs.

Python API Usage Patterns
--------------------------

The previous examples demonstrate several important usage patterns for the Python API, these are:

**Data Access Pattern:**

.. code-block:: python

   # Access the parameter store and container store
   parameter_store = fringe_data_interface.get_parameter_store()
   container_store = fringe_data_interface.get_container_store()

   # Look up an object's UUID via the parameter store, then fetch it
   # from the container store
   vis_uuid = parameter_store.get_by_path("/uuid/visibilities")
   visibility_data = container_store.get_object(vis_uuid)

   # 'plot_data' is only populated during the finalize step, so functions
   # that use it must be registered as a 'python_finalize' operator
   plot_data = fringe_data_interface.get_plot_data()

**Array Manipulation Pattern:**

.. code-block:: python

   # Get the NumPy array view of the underlying C++ data
   vis_array = visibility_data.get_numpy_array()

   # Modify data in-place, indexed by [polprod, channel, time, spectral_point]
   vis_array[pp, ch, :, sp] *= correction_factor

   # Changes are automatically reflected in the C++ data, but DO NOT resize/reshape the numpy arrays!

**Metadata Access Pattern:**

.. code-block:: python

   # Access axis values and axis metadata (JSON tags) for a container
   channel_axis = visibility_data.get_axis(1)
   channel_meta_data = visibility_data.get_axis_metadata(1)
   channel_labels = channel_meta_data["index_labels"]

**Error Handling Pattern:**

.. code-block:: python

   try:
       # Python processing code
       result = process_data(fringe_data_interface)
   except Exception as e:
       # Handle errors gracefully
       print(f"Processing error: {e}")
       return False
