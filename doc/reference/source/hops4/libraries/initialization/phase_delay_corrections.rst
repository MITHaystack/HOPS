Phase and Delay Corrections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The phase and delay correction components provide builders for various types of 
phase and delay correction operators used in VLBI fringe fitting to compensate 
for instrumental and propagation effects.

:hops:`MHO_ManualChannelDelayCorrectionBuilder`
-----------------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ManualChannelDelayCorrectionBuilder`
Primary Functionality                           Builds a manual per-channel pc_delay operator
Key Features                                    | Inherits from MHO_OperatorBuilder and MHO_ChannelQuantity
                                                | ParsePolFromName() method for polarization parsing
                                                | ExtractStationIdentifier() method for station identification
                                                | Handles per-channel delay corrections
=============================================== ====================================================================

The :hops:`MHO_ManualChannelDelayCorrectionBuilder` class builds a manual per-channel 
delay correction operator for applying instrumental delay corrections on a per-channel 
basis. This is essential for compensating for different cable lengths and electronic 
delays in different frequency channels.

The builder includes polarization parsing capabilities and station identification 
methods to properly apply corrections to the appropriate data selection.

:hops:`MHO_ManualChannelPhaseCorrectionBuilder`
-----------------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ManualChannelPhaseCorrectionBuilder`
Primary Functionality                           Builds a manual per-channel pc_phase operator
Key Features                                    | Constructs MHO_ManualChannelPhaseCorrection operator
                                                | Inherits from MHO_OperatorBuilder and MHO_ChannelQuantity
                                                | ParsePolFromName() and ExtractStationIdentifier() methods
                                                | Handles per-channel phase corrections
=============================================== ====================================================================

The :hops:`MHO_ManualChannelPhaseCorrectionBuilder` class builds a manual per-channel 
phase correction operator for applying instrumental phase corrections on a per-channel 
basis. This compensates for phase offsets introduced by different electronic paths 
in the signal processing chain. The builder provides polarization parsing and station identification capabilities 
to ensure corrections are applied to the correct data selection.

:hops:`MHO_ManualPolDelayCorrectionBuilder`
-------------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ManualPolDelayCorrectionBuilder`
Primary Functionality                           Builds a manual per-polarization pc_delay operator
Key Features                                    | Constructs per-polarization manual delay correction operator
                                                | ParsePolFromName() method returns polarization ('X', 'Y', 'R', 'L')
                                                | ExtractStationIdentifier() method for station identification
                                                | Handles per-polarization delay corrections
=============================================== ====================================================================

The :hops:`MHO_ManualPolDelayCorrectionBuilder` class builds a manual per-polarization 
delay correction operator for applying delay corrections based on polarization. 
This is important for compensating for polarization-dependent delays in the signal path
prior to polarization summation.

The builder can handle linear polarizations ('X', 'Y') and circular polarizations 
('R', 'L') and provides station identification capabilities for data selection.

:hops:`MHO_ManualPolPhaseCorrectionBuilder`
-------------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ManualPolPhaseCorrectionBuilder`
Primary Functionality                           Builds a manual per-polarization pc_phase operator
Key Features                                    | Constructs MHO_ManualPolPhaseCorrection operator
                                                | ParsePolFromName() and ExtractStationIdentifier() methods
                                                | Handles per-polarization phase corrections
                                                | Supports linear and circular polarizations
=============================================== ====================================================================

The :hops:`MHO_ManualPolPhaseCorrectionBuilder` class builds a manual per-polarization 
phase correction operator for applying phase corrections based on polarization. 
This compensates for polarization-dependent phase offsets in the signal processing chain,
which are required prior to polarization summation.

The builder provides polarization parsing and station identification methods to 
ensure corrections are applied to the appropriate data selection.

:hops:`MHO_StationDelayCorrectionBuilder`
-----------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_StationDelayCorrectionBuilder`
Primary Functionality                           Builds a station delay correction operator
Key Features                                    | Constructs and initializes MHO_StationDelayCorrection operator
                                                | ParsePolFromName() method for polarization parsing
                                                | ExtractStationIdentifier() method for station identification
                                                | Handles station-level delay corrections
=============================================== ====================================================================

The :hops:`MHO_StationDelayCorrectionBuilder` class builds a station delay correction 
operator for applying delay corrections at the station level. This can be used to compensate
for station-specific delays such as cable delays and local oscillator delays.

:hops:`MHO_MultitonePhaseCorrectionBuilder`
-------------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_MultitonePhaseCorrectionBuilder`
Primary Functionality                           Builds a multitone phase-cal correction operator
Key Features                                    | Handles multitone phase calibration corrections
                                                | ExtractStationMk4ID() method for station identification
                                                | ExtractPCPeriod() method for PC period extraction
                                                | AttachSamplerDelays() and AttachPCToneMask() methods
                                                | GetSamplerDelayKey() method for sampler delay keys
=============================================== ====================================================================

The :hops:`MHO_MultitonePhaseCorrectionBuilder` class builds a multitone phase 
calibration correction operator that handles phase calibration signals generated 
from a pulse-cal system. This is essential for correcting phase variations 
caused by instrumental effects in the signal processing chain (required for VGOS).

The builder provides specialized methods for extracting station identifiers, pcal
periods, and managing sampler delays and tone masks needed for phase calibration corrections.
