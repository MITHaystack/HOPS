Calibration Operators
=====================

MHO_LSBOffset
-------------

======================= ======================================================
Class                   :hops:`MHO_LSBOffset`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.4
Control File Keyword    ``lsb_offset``
======================= ======================================================

Applies LSB phase offset to (the LSB half of) double-sideband channels for specified stations.

MHO_LinearDParCorrection
------------------------

======================= ======================================================
Class                   :hops:`MHO_LinearDParCorrection`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.99
======================= ======================================================

Applies linear delta-parallactic angle correction to visibility data 
for polarization products.


MHO_ManualChannelDelayCorrection
--------------------------------

======================= ======================================================
Class                   :hops:`MHO_ManualChannelDelayCorrection`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.5
Control File Keyword    ``delay_offs_l``, ``delay_offs_r``, ``delay_offs_x``, ``delay_offs_y``
======================= ======================================================


Applies manual channel delay corrections to visibility data for specific 
stations and polarizations.


MHO_ManualChannelPhaseCorrection
--------------------------------

======================= ======================================================
Class                   :hops:`MHO_ManualChannelPhaseCorrection`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.5
Control File Keyword    ``pc_phases_l``, ``pc_phases_r``, ``pc_phases_x``, ``pc_phases_y``
======================= ======================================================


Applies manual channel phase corrections to visibility data for specific 
stations and polarizations.


MHO_ManualPolDelayCorrection
----------------------------

======================= ======================================================
Class                   :hops:`MHO_ManualPolDelayCorrection`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.5
Control File Keyword    ``pc_delay_l``, ``pc_delay_r``, ``pc_delay_x``, ``pc_delay_y``
======================= ======================================================

Applies manual polarization delay corrections to visibility data for specific 
stations and polarizations.

MHO_ManualPolPhaseCorrection
----------------------------

======================= ======================================================
Class                   :hops:`MHO_ManualPolPhaseCorrection`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.5
Control File Keyword    ``pc_phase_offset_l``, ``pc_phase_offset_r``, ``pc_phase_offset_x``, ``pc_phase_offset_y``
======================= ======================================================

Applies manual polarization phase corrections to visibility data for specific 
stations and polarizations.


MHO_MixedPolYShift
------------------

======================= ======================================================
Class                   :hops:`MHO_MixedPolYShift`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.5
Control File Keyword    ``mixed_pol_y_shift``
======================= ======================================================

Applies a 90 degree phase offset to the Y-polarization of each station which the 
linear polarization member of a mixed linear-circular polarization baseline. 
Intended for use in mixed S/X-VGOS geodetic experiments.


MHO_MultitonePhaseCorrection
----------------------------

======================= ======================================================
Class                   :hops:`MHO_MultitonePhaseCorrection`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.1
Control File Keyword    ``pc_mode multitone``
======================= ======================================================

Applies multi-tone phase calibration to visibility data for specific stations 
with configurable averaging periods.


MHO_PolProductSummation
-----------------------

======================= ======================================================
Class                   :hops:`MHO_PolProductSummation`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.99
======================= ======================================================

Performs polarization product summation with support for parallactic angle 
corrections for both reference and remote stations to form the 
pseudo Stokes-I polarization product (VGOS geodesy). Triggered by the command line 
syntax "-P I".


MHO_StationDelayCorrection
--------------------------

======================= ======================================================
Class                   :hops:`MHO_StationDelayCorrection`
Operator Type           Unary
Operator Category       calibration
Argument Data Type      :hops:`visibility_type`
Priority Value          3.5
Control File Keyword    ``station_delay``
======================= ======================================================

Operator to apply a station delay (uniform single delay) to the visibility data 
of all channels for a specified reference or remote station.