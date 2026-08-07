Data Configuration and Initialization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The data configuration and initialization components provide the necessary setup 
and pre-processing functions for fringe fitting operations. These classes handle 
command line parsing, control file processing, and initial parameter setup.

:hops:`MHO_FringeCommandLineParser`
------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringeCommandLineParser`
Primary Functionality                           Collection of helper functions for fringe fitter command line parsing
Key Features                                    | Command line argument parsing
                                                | Baseline and frequency group string parsing
                                                | Control file statement extraction from the command line
                                                | Command line sanity checking
=============================================== ====================================================================

The :hops:`MHO_FringeCommandLineParser` class provides essential helper functions
for parsing the fringe fitter command line. It handles command line argument parsing
(`parse_fourfit_command_line()`), splitting baseline/frequency-group strings
(`parse_baseline_freqgrp()`), extracting inline control file syntax following a `set`
argument (`parse_set_string()`), and performing a post-parse sanity check
(`sanity_check()`).

:hops:`MHO_FringeDataDiscovery`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringeDataDiscovery`
Primary Functionality                           Collection of helper functions for locating scan, baseline, and pass data
Key Features                                    | Root (ovex) file discovery
                                                | Scan directory determination
                                                | Baseline discovery within a scan directory
                                                | Frequency group and polarization-product determination
=============================================== ====================================================================

The :hops:`MHO_FringeDataDiscovery` class provides helper functions for discovering
the scan, baseline, and data pass information needed for fringe fitter start-up. A
single *pass* is made up of a single source, baseline, frequency-group, and
polarization-product. Key functions include `find_associated_root_file()` and
`determine_scans()` for locating scan directories and their root files,
`determine_baselines()` for finding baselines matching a requested pattern within a
scan, and `determine_fgroups_polproducts()` for determining the frequency groups and
polarization products to process for each baseline. It also provides
`determine_passes()` for splitting a set of scans/roots into processing passes.

:hops:`MHO_FringeDataInitializer`
------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringeDataInitializer`
Primary Functionality                           Collection of helper functions for populating scan and container stores
Key Features                                    | Scan data store initialization
                                                | Parameter store population
                                                | Visibility and weight container configuration
                                                | Station data loading and renaming
=============================================== ====================================================================

The :hops:`MHO_FringeDataInitializer` class provides helper functions for
initializing the scan data store and populating the parameter/container stores used
during fringe fitting. Key functions include `initialize_scan_data()` for scan data
store setup, `populate_initial_parameters()` for parameter/scan store population,
`configure_visibility_data()` for validating and preparing visibility/weight
container objects, and station-data loading/renaming helpers.

:hops:`MHO_Mk4InputConverter`
--------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_Mk4InputConverter`
Primary Functionality                           Transparent conversion of legacy mark4 input to HOPS4 format
Key Features                                    | Mark4 scan/experiment directory conversion
                                                | Automatic temporary directory management
                                                | RAII-based cleanup of converted data
=============================================== ====================================================================

The :hops:`MHO_Mk4InputConverter` class transparently converts a legacy mark4
scan or experiment directory into HOPS4 format in a temporary directory before
processing begins, and updates the parameter store's `/cmdline/directory` entry to
point at the converted output. `convert_mk4_input()` performs the conversion, and
`cleanup_mk4_temp_dir()` removes the temporary directory afterward; the accompanying
`MK4TempDirGuard` RAII helper ensures cleanup happens even if the program exits
early.

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
