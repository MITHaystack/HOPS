Calibration Operators
=====================

Calibration operators apply phase, delay, and amplitude corrections to the
visibility data. Within this category, execution order is determined by the
operator priority value (lower priority value = earlier execution). They are
split between user-configurable operators (triggered by control file keywords)
and automatically applied corrections (triggered by data attributes or pipeline
state).

.. toctree::
    :maxdepth: 1

    MHO_AdhocPhaseCorrection
    MHO_CircularFieldRotationCorrection
    MHO_LinearDParCorrection
    MHO_LSBOffset
    MHO_ManualChannelDelayCorrection
    MHO_ManualChannelPhaseCorrection
    MHO_ManualPolDelayCorrection
    MHO_ManualPolPhaseCorrection
    MHO_MixedPolYShift
    MHO_MultitonePhaseCorrection
    MHO_PolProductSummation
    MHO_StationDelayCorrection
