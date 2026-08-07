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
Key Features                                    | Inherits from MHO_OperatorBuilder
                                                | ParsePolFromName() method for polarization parsing
                                                | Uses the inherited GetMatchingStationIdentifiers() mechanism for station identification
                                                | Handles per-channel delay corrections
=============================================== ====================================================================

The :hops:`MHO_ManualChannelDelayCorrectionBuilder` class builds a manual per-channel
delay correction operator for applying instrumental delay corrections on a per-channel
basis. This is necessary for compensating for different cable lengths and electronic
delays in different frequency channels.

The builder includes polarization parsing capabilities and station identification
methods to properly apply corrections to the appropriate data selection.

:hops:`MHO_ManualChannelPhaseCorrectionBuilder`
-----------------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ManualChannelPhaseCorrectionBuilder`
Primary Functionality                           Builds a manual per-channel pc_phase operator
Key Features                                    | Constructs MHO_ManualChannelPhaseCorrection operator
                                                | Inherits from MHO_OperatorBuilder
                                                | ParsePolFromName() method and inherited GetMatchingStationIdentifiers() mechanism
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
                                                | Uses the inherited GetMatchingStationIdentifiers() mechanism for station identification
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
                                                | ParsePolFromName() method and inherited GetMatchingStationIdentifiers() mechanism
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
                                                | Uses the inherited GetMatchingStationIdentifiers() mechanism for station identification
                                                | Handles station-level delay corrections
=============================================== ====================================================================

The :hops:`MHO_StationDelayCorrectionBuilder` class builds a station delay correction
operator for applying delay corrections at the station level. This can be used to compensate
for station-specific delays such as cable delays and local oscillator delays. The
operator is applied uniformly to the whole station, without regard to polarization.

:hops:`MHO_MultitonePhaseCorrectionBuilder`
-------------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_MultitonePhaseCorrectionBuilder`
Primary Functionality                           Builds a multitone phase-cal correction operator
Key Features                                    | Handles multitone phase calibration corrections
                                                | ExtractStationCode() method returns the 2-character station code for station identification
                                                | ExtractPCPeriod() method for PC period extraction
                                                | AttachSamplerDelays() and AttachPCToneMask() methods
                                                | GetSamplerDelayKey() method for sampler delay keys
=============================================== ====================================================================

The :hops:`MHO_MultitonePhaseCorrectionBuilder` class builds a multitone phase
calibration correction operator that handles phase calibration signals generated
from a pulse-cal system. This is necessary for correcting phase variations
caused by instrumental effects in the signal processing chain (required for VGOS).

The builder provides specialized methods for extracting station identifiers, pcal
periods, and managing sampler delays and tone masks needed for phase calibration corrections.

:hops:`MHO_AdhocFlaggingBuilder`
------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_AdhocFlaggingBuilder`
Primary Functionality                           Builds an ad-hoc flagging operator from control file parameters
Key Features                                    | Triggered by an `adhoc_flag_file <path>` control statement
                                                | Supports per-station flag files via station-conditional control blocks
                                                | Maintains a single shared operator, updating ref/rem file paths as statements are processed
=============================================== ====================================================================

The :hops:`MHO_AdhocFlaggingBuilder` class builds an ad-hoc flagging operator that
applies a user-supplied flag file to a station's data. It is triggered by an
`adhoc_flag_file <path>` compound statement in the control file, which may appear
inside a station-conditional block (`if station X ...`) to set the flag file for a
specific station, or outside any condition to apply the same file to both stations.
Because a baseline has two stations, the builder maintains a single
`adhoc_flagging` operator in the toolbox and updates its reference/remote file
paths as each control statement is processed.

:hops:`MHO_AdhocPhaseCorrectionBuilder`
-------------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_AdhocPhaseCorrectionBuilder`
Primary Functionality                           Builds an ad-hoc phase correction operator from control file parameters
Key Features                                    | Triggered by an `adhoc_phase <algorithm_type>` control statement
                                                | Supports sinewave, polynomial, and file-based correction modes
                                                | Supports per-station override files via `adhoc_file`/`adhoc_file_chans`
=============================================== ====================================================================

The :hops:`MHO_AdhocPhaseCorrectionBuilder` class builds an ad-hoc phase correction
operator that applies a user-specified phase correction model to a station's data.
It is triggered by an `adhoc_phase <algorithm_type>` compound statement in the
control file, where `algorithm_type` selects the correction mode: `sinewave`,
`polynomial`, or `file`. Auxiliary parameters (reference time, period, amplitude, or
polynomial coefficients) are drawn from the parameter store on a per-station basis,
and per-station file-based overrides can be supplied via `adhoc_file`/
`adhoc_file_chans`.
