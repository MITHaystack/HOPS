Python Examples
~~~~~~~~~~~~~~~

The Python examples demonstrate a couple practical applications of the Python bindings 
for solving real-world VLBI data processing problems. These examples show how 
to use the Python API for custom calibration, data correction, and analysis tasks.

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
to correct known instrumental phase jumps in visibility data involving the NOEMA telescope array.

**Implementation Details:**

.. code-block:: python

   def fix_noema_jumps(fringe_data_interface):
       """
       Corrects phase jumps in NOEMA telescope data based on known 
       instrumental characteristics and frequency-dependent effects.
       """
       # Access visibility data through the fringe data interface
       visibility_data = fringe_data_interface.get_visibility_data()
       
       # Apply channel-specific phase corrections
       for channel in range(visibility_data.get_num_channels()):
           # Identify phase jumps based on frequency
           if is_phase_jump_channel(channel):
               # Apply correction to both polarizations
               apply_phase_correction(visibility_data, channel)

**Key Features:**
- **Direct Data Access**: Uses fringe_data_interface to access visibility arrays
- **Channel-wise Processing**: Applies corrections on a per-channel basis
- **Polarization Handling**: Manages corrections for different polarization products
- **Frequency Analysis**: Uses frequency information to identify problematic channels

**Applications:**
- Correcting known instrumental phase jumps in NOEMA data
- Preprocessing data before fringe fitting
- Improving data quality for challenging observational conditions

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

The phase calibration generation example shows how to extract phase calibration 
corrections from fringe fitting results and generate output compatible with 
the fourfit control file format.

**Implementation Details:**

.. code-block:: python

   def generate_pcphases(fringe_data_interface):
       """
       Generates phase calibration corrections from fringe fit results
       by analyzing phase residuals across frequency channels.
       """
       # Access plot data from the fringe fitting results
       plot_data = fringe_data_interface.get_plot_data()
       
       # Extract phase residuals for each channel
       phase_residuals = extract_phase_residuals(plot_data)
       
       # Calculate circular mean phases
       pc_phases = calculate_circular_mean_phases(phase_residuals)
       
       # Generate fourfit-compatible output
       generate_fourfit_output(pc_phases)

**Key Processing Steps:**

1. **Data Extraction**: Retrieves fringe fitting results and plot data
2. **Phase Analysis**: Analyzes phase residuals across frequency channels
3. **Statistical Processing**: Computes circular mean phases for each channel
4. **Output Generation**: Creates fourfit-compatible phase calibration files

**Mathematical Operations:**
- **Circular Statistics**: Proper handling of phase angles using circular means
- **Residual Analysis**: Extraction of phase residuals from fit results
- **Channel Mapping**: Correlation of phase corrections with frequency channels
- **Error Propagation**: Handling of uncertainties in phase measurements

**Output Format:**
The generated output is compatible with fourfit control files and can be used 
for subsequent processing:

.. code-block:: text

   * Phase calibration corrections generated from fringe fit
   pc_phases ref_station rem_station
   pc_phases A B  0.0 15.2 -23.1 8.7 ...
   pc_phases A C  5.1 -12.3 18.9 -6.4 ...

**Applications:**
- **Automated Calibration**: Generation of phase calibration corrections
- **Data Quality Assessment**: Analysis of phase stability across channels
- **Iterative Processing**: Refinement of calibration in multi-pass processing
- **Custom Calibration**: Development of specialized calibration algorithms

Python API Usage Patterns
--------------------------

The examples demonstrate several important usage patterns for the Python API:

**Data Access Pattern:**

.. code-block:: python

   # Access different data interfaces
   fringe_data = fringe_data_interface.get_fringe_data()
   container_store = fringe_data_interface.get_container_store()
   parameter_store = fringe_data_interface.get_parameter_store()
   
   # Access specific data containers
   visibility_data = fringe_data.get_visibility_data()
   plot_data = fringe_data.get_plot_data()

**Array Manipulation Pattern:**

.. code-block:: python

   # Get NumPy arrays for efficient processing
   vis_array = visibility_data.get_numpy_array()
   
   # Modify data in-place
   vis_array[channel_mask] *= correction_factor
   
   # Changes are automatically reflected in C++ data

**Metadata Access Pattern:**

.. code-block:: python

   # Access metadata through Python dictionaries
   metadata = container.get_metadata()
   station_info = metadata['stations']
   frequency_info = metadata['frequencies']

**Error Handling Pattern:**

.. code-block:: python

   try:
       # Python processing code
       result = process_data(fringe_data_interface)
   except Exception as e:
       # Handle errors gracefully
       print(f"Processing error: {e}")
       return False

**Development Guidelines:**

1. **Efficiency**: Use NumPy arrays for numerical operations
2. **Memory Management**: Leverage zero-copy access where possible
3. **Type Safety**: Utilize automatic type checking and conversion
4. **Error Handling**: Implement robust exception handling
5. **Documentation**: Provide clear function documentation
6. **Testing**: Include validation and testing code

**Integration with HOPS4 Pipeline:**
The Python examples show how custom Python functions can be integrated 
into the HOPS4 processing pipeline through the operator framework, enabling:

- **Custom Preprocessing**: Data correction before fringe fitting
- **Custom Analysis**: Specialized analysis of fringe fitting results
- **Quality Control**: Automated data quality assessment
- **Calibration**: Custom calibration algorithm implementation
- **Visualization**: Custom plotting and data visualization
- **Automation**: Automated processing and decision making
