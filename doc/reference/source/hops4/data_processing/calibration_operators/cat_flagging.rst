Flagging Operators
==================

Flagging operators zero-out or mask problematic data in the visibility array. They
run after selection and before the calibration category. Unlike selection operators
which remove data based on weight thresholds, flagging operators act on the
visibility amplitudes directly (e.g. notching spectral channels, blocking the DC
bin, applying ad hoc flag files).

.. toctree::
    :maxdepth: 1

    MHO_AdhocFlagging
    MHO_DCBlock
    MHO_NotchComb
    MHO_Notches
    MHO_Passband
