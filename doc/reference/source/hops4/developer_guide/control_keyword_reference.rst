Control File Keyword Reference
==============================

This page documents every control file keyword recognized by ``fourfit4``.
Keywords are grouped by their function in the processing pipeline.

**Supported** reflects current HOPS4 implementation status.
**EHT / VGOS** indicates which experiment type requires the keyword (``EHT``, ``VGOS``, or ``BOTH``).

.. contents:: Sections
   :local:
   :depth: 1

Conditional Statements
----------------------

Conditional blocks allow control file statements to be applied
selectively based on baseline, station, source, or scan properties.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``else``
     -
     - BOTH
     - No
     -
     - Never implemented in the original fourfit
   * - ``if``
     - list of strings
     - BOTH
     - Yes
     - MHO_ControlConditionEvaluator and MHO_ControlFileParser
     -



Labeling Operators
------------------

Labeling operators assign identifiers (channel labels, sampler indices, polarization labels) to the data. They execute first in the operator pipeline, before any calibration is applied.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``chan_ids``
     - channel_names: string,  channel_frequencies: list of floats
     - BOTH
     - Yes
     - MHO_ChannelLabeler
     -
   * - ``python_labeling``
     - module_path: string,  function_name: string,  [override_priority: float]
     - BOTH
     - Yes
     - MHO_PythonOperatorBuilder
     - Requires build with HOPS_ENABLE_EMBEDDED_PYTHON=ON; priority 0.2
   * - ``samplers``
     - fixed-length list of strings
     - BOTH (VGOS mainly)
     - Yes
     - MHO_SamplerLabeler
     -
   * - ``swap_pol_labels``
     - pol1: string,  pol2: string
     - BOTH
     - Yes
     - MHO_PolarizationRelabeler / MHO_PolarizationRelabelerBuilder
     - priority 0.1



Flagging and Selection
----------------------

These keywords mark or exclude data based on quality criteria, spectral notches, passband limits, or time/frequency selection.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``dc_block``
     - true \| false
     - BOTH
     - Yes
     - MHO_DCBlock
     - Needs testing (check XP spectrum agains ff3)
   * - ``freqs``
     - logical-intersection list of strings
     - BOTH
     - Yes
     - MHO_DataSelectionBuilder and MHO_SelectRepack
     - We need to debug the case where freqs selects down to just 1 channel
   * - ``min_weight``
     - float
     - BOTH
     - Yes
     -
     - Not fully tested, appears to be working
   * - ``notch_comb``
     - offset: float,  period: float,  width: float
     - BOTH
     - Yes
     - MHO_NotchComb / MHO_NotchCombBuilder
     - priority 4.5
   * - ``notches``
     - list of floats
     - BOTH
     - Yes
     - MHO_Notches
     - SNR agrees with fourfit3, more testing needed
   * - ``passband``
     - list of floats
     - BOTH
     - Yes
     - MHO_Passband
     - SNR agrees with fourfit3, more testing needed
   * - ``python_flagging``
     - module_path: string,  function_name: string,  [override_priority: float]
     - BOTH
     - Yes
     - MHO_PythonOperatorBuilder
     - Requires build with HOPS_ENABLE_EMBEDDED_PYTHON=ON; priority 2.9
   * - ``skip``
     - true \| false
     - BOTH
     - Yes
     -
     -
   * - ``start``
     - integer
     - BOTH
     - Yes
     - MHO_DataSelectionBuilder and MHO_SelectRepack
     - integer sec within scan selection enabled, minute-past-the-hour is not yet supported
   * - ``stop``
     - integer
     - BOTH
     - Yes
     - MHO_DataSelectionBuilder and MHO_SelectRepack
     - integer sec within scan selection enabled, minute-past-the-hour is not yet supported



Calibration Operators
---------------------

Calibration operators apply phase, delay, and amplitude corrections to the visibility data. Execution order within this category is determined by the operator priority value.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``adhoc_file``
     - string
     - BOTH
     - Yes
     - MHO_AdhocPhaseCorrection
     - Tested for USB
   * - ``adhoc_file_chans``
     - string
     - BOTH
     - Yes
     - MHO_AdhocPhaseCorrection
     - Tested for USB
   * - ``adhoc_flag_file``
     - flag_file: string
     - EHT
     - Yes
     - MHO_AdhocFlagging
     - Tested for USB
   * - ``adhoc_period``
     - float
     - BOTH
     - Yes
     - MHO_AdhocPhaseCorrection
     - Tested for USB
   * - ``adhoc_phase``
     - algorithm_type: string
     - EHT
     - Yes
     - MHO_AdhocPhaseCorrection
     - Tested for USB
   * - ``adhoc_poly``
     - list of floats
     - EHT?
     - Yes
     - MHO_AdhocPhaseCorrection
     - Tested for USB
   * - ``adhoc_tref``
     - float
     - BOTH
     - Yes
     - MHO_AdhocPhaseCorrection
     - Tested for USB
   * - ``delay_offs``
     - channel_names: string,  pc_delays: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelDelayCorrection
     -
   * - ``delay_offs_l``
     - channel_names: string,  pc_delays: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelDelayCorrection
     - Needs further testing! -- phase_shift compensation for VGOS not yet implemented
   * - ``delay_offs_r``
     - channel_names: string,  pc_delays: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelDelayCorrection
     - Needs further testing! -- phase_shift compensation for VGOS not yet implemented
   * - ``delay_offs_x``
     - channel_names: string,  pc_delays: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelDelayCorrection
     - Needs further testing! -- phase_shift compensation for VGOS not yet implemented
   * - ``delay_offs_y``
     - channel_names: string,  pc_delays: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelDelayCorrection
     - Needs further testing! -- phase_shift compensation for VGOS not yet implemented
   * - ``mount_type``
     - string
     - EHT
     - Yes
     -
     - Supports apply field rotation to R/L pols for cassegrain, and nasmyth left/right telescopes, needs testing!
   * - ``pc_delay_l``
     - float
     - BOTH
     - Yes
     - MHO_ManualPolDelayCorrection
     - Needs further testing!
   * - ``pc_delay_r``
     - float
     - BOTH
     - Yes
     - MHO_ManualPolDelayCorrection
     - Needs further testing!
   * - ``pc_delay_x``
     - float
     - BOTH
     - Yes
     - MHO_ManualPolDelayCorrection
     - Needs further testing!
   * - ``pc_delay_y``
     - float
     - BOTH
     - Yes
     - MHO_ManualPolDelayCorrection
     - Needs further testing!
   * - ``pc_mode``
     - string
     - VGOS (multitone) BOTH (manual)
     - Yes
     -
     - manual pc supported, multitone implemented (but only tested on LSB data!)
   * - ``pc_period``
     - integer
     - BOTH
     - Yes
     -
     -
   * - ``pc_phase_offset_l``
     - float
     - BOTH
     - Yes
     - MHO_ManualPolPhaseCorrection
     - Needs further testing!
   * - ``pc_phase_offset_r``
     - float
     - BOTH
     - Yes
     - MHO_ManualPolPhaseCorrection
     - Needs further testing!
   * - ``pc_phase_offset_x``
     - float
     - BOTH
     - Yes
     - MHO_ManualPolPhaseCorrection
     - Needs further testing!
   * - ``pc_phase_offset_y``
     - float
     - BOTH
     - Yes
     - MHO_ManualPolPhaseCorrection
     - Needs further testing!
   * - ``pc_phases``
     - channel_names: string,  pc_phases: list of floats
     - BOTH
     - Yes
     - MHO_ManualPolPhaseCorrection
     -
   * - ``pc_phases_l``
     - channel_names: string,  pc_phases: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelPhaseCorrection
     - Tested for LSB/USB but not DSB
   * - ``pc_phases_r``
     - channel_names: string,  pc_phases: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelPhaseCorrection
     - Tested for LSB/USB but not DSB
   * - ``pc_phases_x``
     - channel_names: string,  pc_phases: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelPhaseCorrection
     - Tested for LSB/USB but not DSB
   * - ``pc_phases_y``
     - channel_names: string,  pc_phases: list of floats
     - BOTH
     - Yes
     - MHO_ManualChannelPhaseCorrection
     - Tested for LSB/USB but not DSB
   * - ``python_calibration``
     - module_path: string,  function_name: string,  [override_priority: float]
     - BOTH
     - Yes
     - MHO_PythonOperatorBuilder
     - Requires build with HOPS_ENABLE_EMBEDDED_PYTHON=ON; priority 3.9
   * - ``python_finalize``
     - module_path: string,  function_name: string,  [override_priority: float]
     - BOTH
     - Yes
     - MHO_PythonOperatorBuilder
     - Requires build with HOPS_ENABLE_EMBEDDED_PYTHON=ON; priority 9.9
   * - ``station_delay``
     - float
     - BOTH
     - Yes
     - MHO_StationDelayCorrection
     - delay is applied directly to visibilities (note that the implementation is not the same as ff3, where it is applied during pc_delay amb resoluiton)



Fringe Fitter Parameters
------------------------

These parameters control the fringe-fitting search: delay and delay-rate windows, coherence time, reference frequency, phase calibration mode, and related station-specific settings.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``dr_win``
     - list of floats
     - BOTH
     - Yes
     -
     - Needs testing
   * - ``est_pc_manual``
     - integer
     - BOTH
     - Yes
     - MHO_EstimatePCManual
     - Needs additional testing (there may be a bug in the HOPS3 implmentation w.r.t to channel label mapping)
   * - ``gen_cf_record``
     - true \| false
     - BOTH
     - Yes
     -
     - always true (however we may want to allow the CF to be discarded too). AEN request -- provide option to store CF with comments as well
   * - ``lsb_offset``
     - float
     - BOTH
     - Yes
     - This is a double-side band data feature
     -
   * - ``mb_win``
     - list of floats
     - BOTH
     - Yes
     -
     - Needs testing
   * - ``mbd_anchor``
     - string
     - BOTH
     - Yes
     -
     -
   * - ``optimize_closure``
     - true \| false
     - BOTH
     - Yes
     -
     -
   * - ``ref_freq``
     - float
     - BOTH
     - Yes
     - MHO_DelayRate, MHO_InterpolateFringePeak, MHO_ComputePlotData
     - Need to document everywhere this field is used
   * - ``sb_win``
     - list of floats
     - BOTH
     - Yes
     -
     - Needs testing
   * - ``spectral_line``
     - true \| false
     - EHT
     - No, still in testing
     -
     - Read via /control/fit/spectral_line
   * - ``spectral_line_freq_win``
     - list of floats
     - EHT
     - No, still in testing
     -
     - Read via /control/fit/spectral_line_freq_win
   * - ``t_cohere``
     - float
     - EHT?
     - Yes
     - MBDelaySearch (and OpenMP/CUDA) extensions
     - This is passed as a parameter, which triggers the associated behavior in MBDelaySearch, it is not a standalone operator
   * - ``weak_channel``
     - float
     - BOTH
     - Yes
     -
     -



VGOS-Only Parameters
--------------------

These parameters are specific to VGOS (geodetic) broadband observations, including ionospheric correction, sampler delays, and tone masking.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``ion_npts``
     - integer
     - VGOS
     - Yes
     -
     -
   * - ``ion_smooth``
     - true \| false
     - VGOS
     - Yes
     -
     -
   * - ``ion_win``
     - list of floats
     - VGOS
     - Yes
     -
     -
   * - ``ionosphere``
     - float
     - VGOS
     - Yes
     -
     - value read in and mapped to station ID
   * - ``mixed_pol_yshift90``
     - true \| false
     - VGOS/Mixedmode
     - Yes
     -
     -
   * - ``pc_amp_hcode``
     - float
     - VGOS
     - Yes
     -
     -
   * - ``pc_tonemask``
     - channel_names: string,  tone_masks: ?
     - VGOS
     - Yes
     -
     - Make sure this works with USB data too
   * - ``sampler_delay_l``
     - list of floats
     - VGOS
     - Yes
     -
     -
   * - ``sampler_delay_r``
     - list of floats
     - VGOS
     - Yes
     -
     -
   * - ``sampler_delay_x``
     - list of floats
     - VGOS
     - Yes
     -
     -
   * - ``sampler_delay_y``
     - list of floats
     - VGOS
     - Yes
     -
     -



Global Configuration Parameters
-------------------------------

Global configuration parameters affect overall pipeline behavior and are not specific to a single baseline or station.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``adhoc_amp``
     - float
     - BOTH
     - Yes
     -
     -
   * - ``plot_backend``
     - string
     - BOTH
     - Yes
     -
     -
   * - ``python_custom_plot``
     - module_path: string,  function_name: string
     - BOTH
     - Yes
     - MHO_PythonOperatorBuilder
     - Requires build with HOPS_ENABLE_EMBEDDED_PYTHON=ON



Python Extension Operators
--------------------------

Python extension operators invoke user-supplied Python functions at specific points in the pipeline (pre-fit, post-fit, calibration, flagging, labeling, finalization, and custom plotting). See the Python Plugin User Guide for details.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``python_postfit``
     - module_path: string,  function_name: string,  [override_priority: float]
     - BOTH
     - Yes
     - MHO_PythonOperatorBuilder
     - Requires build with HOPS_ENABLE_EMBEDDED_PYTHON=ON; priority 8.9
   * - ``python_prefit``
     - module_path: string,  function_name: string,  [override_priority: float]
     - BOTH
     - Yes
     - MHO_PythonOperatorBuilder
     - Requires build with HOPS_ENABLE_EMBEDDED_PYTHON=ON; priority 7.9



Deprecated / Not Supported
--------------------------

The following keywords are parsed for backward compatibility but are **not** implemented in HOPS4. They will be silently ignored or rejected.

.. list-table::
   :header-rows: 1
   :widths: 12 28 8 8 20 20

   * - Keyword
     - Format
     - EHT / VGOS
     - Supported
     - Implementation Class
     - Notes
   * - ``avxplopt``
     -
     -
     - No
     -
     - reserved, not yet implemented
   * - ``avxpzoom``
     -
     -
     - No
     -
     - reserved, not yet implemented
   * - ``dec_offset``
     - deprecated
     - BOTH
     - No
     -
     - this keyword is parsed in c-lib, but does nothing
   * - ``fmatch_bw_pct``
     -
     - BOTH
     - No?
     -
     -
   * - ``gates``
     - deprecated
     - BOTH
     - No
     -
     - part of frequency switching mode feature
   * - ``index``
     - deprecated
     -
     - No
     -
     - MK4 correlator specific
   * - ``interpolator``
     -
     - BOTH
     - deprecated
     - MHO_InterpolateFringePeak
     - simul' only, iterate is not implemented
   * - ``max_parity``
     -
     -
     - No
     -
     -
   * - ``pc_freqs``
     -
     - BOTH
     - No
     -
     -
   * - ``period``
     - deprecated
     - BOTH
     - No
     -
     - part of frequency switching mode feature
   * - ``plot_data_dir``
     -
     - BOTH
     - No
     -
     - Generally speaking this operator is not needed in hops4 as the plot data is already fully output
   * - ``ra_offset``
     - deprecated
     - BOTH
     - No
     -
     - this keyword is parsed in c-lib, but does nothing
   * - ``switched``
     - deprecated
     -
     - No
     -
     - part of frequency switching mode feature
   * - ``use_samples``
     - true \| false
     - BOTH
     - No
     -
     - Normalizes data according to sampler statistics (This a mark4 correlator only feature)
   * - ``vbp_coeffs``
     -
     -
     - No
     -
     - reserved, not yet implemented
   * - ``vbp_correct``
     -
     -
     - No
     -
     - reserved, not yet implemented
   * - ``vbp_file``
     -
     -
     - No
     -
     - reserved, not yet implemented
   * - ``vbp_fit``
     -
     -
     - No
     -
     - reserved, not yet implemented
   * - ``x_crc``
     - `keep' or `discard'
     -
     - No
     -
     -
   * - ``x_slip_sync``
     - `keep'
     -
     - No
     -
     -
   * - ``y_crc``
     - `keep' or `discard'
     -
     - No
     -
     -
   * - ``y_slip_sync``
     - `keep'
     -
     - No
     -
     -
