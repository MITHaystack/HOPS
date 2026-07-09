Labeling Operators
==================

Labeling operators run first in the operator pipeline (priority < 1). They assign
channel identifiers, sampler indices, and polarization labels to the visibility
and weight data before any calibration or selection is applied. They are triggered
by the corresponding control file keywords.

.. toctree::
    :maxdepth: 1

    MHO_ChannelLabeler
    MHO_SamplerLabeler
    MHO_PolarizationRelabeler
    MHO_PolarizationProductRelabeler
