Computation and Utilities
~~~~~~~~~~~~~~~~~~~~~~~~~

The computation utilities provide essential helper functions and calculations 
for fringe fitting operations, including SNR calculations, error analysis, 
and plot data computation.

:hops:`MHO_BasicFringeInfo`
---------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_BasicFringeInfo`
Primary Functionality                           Static helper functions for fringe information calculations
Key Features                                    | SNR calculations with various correction factors
                                                | Error calculations for delays and rates
                                                | Phase calculations and corrections
                                                | Theoretical RMS calculations
=============================================== ====================================================================

The :hops:`MHO_BasicFringeInfo` class provides a collection of static helper functions 
for computing fringe information and parameters. This includes the signal-to-noise ration (SNR) and
probability-of-false-detection (PFD) calculation, error coding/flagging functions, and phase correction calculations.
Some functions include `calculate_snr()` for signal-to-noise ratio calculations, 
`calculate_mbd_no_ion_error()` for multi-band delay error estimation, and 
`calculate_pfd()` for probability of false detection calculations.

:hops:`MHO_BasicFringeUtilities`
--------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_BasicFringeUtilities`
Primary Functionality                           Helper functions for fringe fitting organization
Key Features                                    | Sideband average calculations
                                                | Residual phase calculations
                                                | Sampling rate determination
                                                | SNR correction factors
=============================================== ====================================================================

The :hops:`MHO_BasicFringeUtilities` class provides organizational helper functions 
for fringe fitting operations. It includes sideband averaging, residual phase 
calculations, and SNR correction factor computations.

Some key functions include `calculate_sbavg()` for sideband average calculations, 
`calculate_fringe_solution_info()` for fringe solution parameters, and 
`calculate_snr_correction_factor()` for bandwidth corrections 
(if notches or passband have been used).

:hops:`MHO_ComputePlotData`
---------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ComputePlotData`
Primary Functionality                           Fringe plot information computation
Key Features                                    | Multi-band delay (MBD) and single-band delay (SBD) calculations
                                                | Cross-power spectrum computation
                                                | Frequency and time domain RMS calculations
                                                | Fringe quality and error code generation
=============================================== ====================================================================

The :hops:`MHO_ComputePlotData` class computes fringe plot information required for 
visualization and analysis. It provides the main set of calculations needed for 
power spectra, RMS values, and quality assessment metrics that form the basis of the 
fringe plot.

Some key functions include `calc_mbd()`, `calc_sbd()`, and `calc_dr()` for fringe 
amplitude vs. fit paramater plots. It also provides `calc_freqrms()` and `calc_timerms()` 
for RMS calculations, and `calc_quality_code()` and `calc_error_code()` for 
fringe quality assessment.
