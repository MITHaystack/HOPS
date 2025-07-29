Non-configurable Operators
===========================


MHO_DelayRate
-------------

=========================== ======================================================
Class                       :hops:`MHO_DelayRate`
Operator Type               Binary
Operator Category           Non-configurable (search)
Input Argument Data Types   :hops:`visibility_type`, :hops:`weight_type`
Output Argument Data Type   :hops:`sbd_type`
=========================== ======================================================

Performs the transform to the delay-rate space by applying a FFT. This operator 
also applies the data-weights. The FFT is done with zero-padding and a cyclic rotation
to center the delay rate at zero. Linear interpolation over the delay-rate values 
is applied to the output.

MHO_DoubleSidebandChannelLabeler
--------------------------------

======================= ======================================================
Class                   :hops:`MHO_DoubleSidebandChannelLabeler`
Operator Type           Unary
Operator Category       Non-configurable (labeling)
Argument Data Type      :hops:`visibility_type` or :hops:`weight_type`
======================= ======================================================

Detects adjacent LSB/USB channels pairs which share the same sky-frequency and 
bandwidth. These are then marked as 'double-sideband' channels so they can 
receive the legacy treatment.


MHO_InterpolateFringePeak
-------------------------

======================= ======================================================
Class                   :hops:`MHO_InterpolateFringePeak`
Operator Type           Special
Operator Category       Non-configurable (search)
Argument Data Type      :hops:`visibility_type`
======================= ======================================================

Implements fine interpolation about the fringe peak 
(see interp.c and max555.c code).


MHO_IonosphericPhaseCorrection
------------------------------

======================= ======================================================
Class                   :hops:`MHO_IonosphericPhaseCorrection`
Operator Type           Unary
Operator Category       Non-configurable (search)
Argument Data Type      :hops:`visibility_type`
Control File Keyword    ``ion_n_pts``, ``ion_smooth``, ``ion_win``, ``ionosphere``
======================= ======================================================

Applies differential ionospheric phase correction to visibility data based on 
differential TEC values.

MHO_MBDelaySearch
-----------------

======================= ======================================================
Class                   :hops:`MHO_MBDelaySearch`
Operator Type           Inspecting
Operator Category       Non-configurable (search)
Argument Data Type      :hops:`visibility_type`
======================= ======================================================

Implements the coarse MBD/SBD/DR search, see search.c. Performs coarse 
multi-band delay search operations.


MHO_NormFX
----------

======================= ======================================================
Class                   :hops:`MHO_NormFX`
Operator Type           Unary
Operator Category       Non-configurable (search)
Argument Data Type      :hops:`visibility_type`
======================= ======================================================

Wrapper class for the interface which provides the functionality of the original
norm_fx.c operation. This is a unary operator on visibilities that accepts 
weights as an additional parameter.

MHO_MixedSidebandNormFX
-----------------------

======================= ======================================================
Class                   :hops:`MHO_MixedSidebandNormFX`
Operator Type           Unary
Operator Category       Non-configurable (search)
Argument Data Type      :hops:`visibility_type`
======================= ======================================================

Implements a subset of the functionality found in norm_fx.c, mainly the transform 
from frequency to delay space for data that contains channels with a mixture of 
upper and lower (USB/LSB) sideband data. Preserves the use of the extra padding 
factor (8x) followed by factor-of-2 sub-sampling during the transform to delay-space.


MHO_SingleSidebandNormFX
------------------------

======================= ======================================================
Class                   :hops:`MHO_SingleSidebandNormFX`
Operator Type           Unary
Operator Category       Non-configurable (search)
Argument Data Type      :hops:`visibility_type`
======================= ======================================================

Implements a subset of the functionality found in norm_fx.c, mainly the transform 
from frequency to delay space for data that contains channels of only a single 
sideband type (either USB or LSB). This implementation reduces the required zero
padding factor by 2x over the original norm_fx.c implemention.

MHO_PhaseCalibrationTrim
------------------------

======================= ======================================================
Class                   :hops:`MHO_PhaseCalibrationTrim`
Operator Type           Unary
Operator Category       Non-configurable (selection)
Argument Data Type      :hops:`multitone_pcal_type`
Required Input          :hops:`visibility_type`
======================= ======================================================

Trims the time range of the pcal data to match that of the visibilities. If the 
pcal or visibilty data is missing the 'start_time' tag, it is implicitly assumed 
to be the same.


MHO_SBDTableGenerator
---------------------

========================== ======================================================
Class                      :hops:`MHO_SBDTableGenerator`
Operator Type              Transforming
Operator Category          Non-configurable (utility)
Input Argument Data Type   :hops:`visibility_type`
Output Argument Data Type  :hops:`sbd_type`
========================== ======================================================

Implements the conversion of the input visibility array into a data object which can
be transformed into single-band delay space (via the NormFX operators).


MHO_VisibilityChannelizer
-------------------------

========================== ======================================================
Class                      :hops:`MHO_VisibilityChannelizer`
Operator Type              Transforming
Operator Category          Non-configurable (utility)
Input Argument Data Type   :hops:`uch_visibility_store_type`
Output Argument Data Type  :hops:`visibility_store_type`
========================== ======================================================

Collects unchannelized (3d) visibility data and groups by channel into 4d object.
All channels must be of equal size. This is a utility operator only used by the 
MK4Interface library when converting Mark4 visibility objects to HOPS4 format.

MHO_WeightChannelizer
---------------------

========================== ======================================================
Class                      :hops:`MHO_WeightChannelizer`
Operator Type              Transforming
Operator Category          Non-configurable (utility)
Input Argument Data Type   :hops:`uch_weight_store_type`
Output Argument Data Type  :hops:`weight_store_type`
========================== ======================================================

Collects unchannelized (3d) weight data and groups by channel into 4d object.
All channels must be of equal size. This is a utility operator only used by the 
MK4Interface library when converting Mark4 weight objects to HOPS4 format.
