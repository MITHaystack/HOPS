Data Configuration and Initialization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The data configuration and initialization components provide the necessary setup 
and pre-processing functions for fringe fitting operations. These classes handle 
command line parsing, control file processing, and initial parameter setup.

:hops:`MHO_BasicFringeDataConfiguration`
----------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_BasicFringeDataConfiguration`
Primary Functionality                           Collection of helper functions for fringe fitter start-up
Key Features                                    | Command line argument parsing
                                                | Baseline and frequency group processing
                                                | Scan and data pass determination
                                                | Operator initialization and execution
=============================================== ====================================================================

The :hops:`MHO_BasicFringeDataConfiguration` class provides essential helper functions 
for initializing fringe fitter operations. It handles command line argument parsing, 
determines processing passes and initializes scan data structures. A single *pass* 
is made up of a single source, baseline, frequency-group, and polarization-product. 
Key functions include `parse_fourfit_command_line()` for argument processing and 
`initialize_scan_data()` for scan data store setup.

:hops:`MHO_FringeControlInitialization`
---------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringeControlInitialization`
Primary Functionality                           Control file processing and initialization
Key Features                                    | Control file processing and statement management
                                                | Default operator definitions
                                                | Polarization product handling
                                                | Ionospheric search detection
=============================================== ====================================================================

The :hops:`MHO_FringeControlInitialization` class handles the processing of control 
files and initialization of fringe fitting parameters. It manages control file 
statements, adds default operators, and determines processing requirements such 
as whether or not an ionospheric dTEC search is needed.

The class provides functions like `process_control_file()` for control file handling 
and `need_ion_search()` for determining ionospheric processing requirements.

:hops:`MHO_InitialFringeInfo`
-----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_InitialFringeInfo`
Primary Functionality                           A priori parameter population for fringe fitting
Key Features                                    | Default parameter initialization
                                                | Reference frequency configuration
                                                | Clock model calculations
                                                | Pre-calculation of derived quantities
=============================================== ====================================================================

The :hops:`MHO_InitialFringeInfo` class provides helper functions for populating 
the parameter store with *a priori* information known before fringe fitting begins. 
This includes setting default parameters, configuring the reference frequency, and 
calculating *a priori* clock models.

Key functions include `set_default_parameters_minimal()` for basic parameter setup 
and `calculate_clock_model()` for clock model initialization.
