..  _CalibrationOperatorReference:

Calibration Operator Reference
===============================

The calibration operator reference documents every operator available in the
HOPS4 fringe-fitting pipeline. Operators are broadly divided into two groups:

**Configurable operators** are triggered by, or have parameters accessible
through, user-issued control file statements or ``fourfit4`` command line
arguments.

**Non-configurable operators** compartmentalize portions of the internal
fringe-finding algorithm and are not directly accessible from the control file.
They are triggered automatically by the pipeline based on data attributes or
pipeline state.

The configurable operators are organized into functional categories. During
runtime each category is initialized and executed in the following order:

#. Labeling
#. Selection
#. Flagging
#. Calibration
#. Pre-fit  *(user Python hooks)*
#. Post-fit *(user Python hooks)*
#. Finalize

Within each category, execution order is determined by the operator's priority
value - a lower value means earlier execution.

.. toctree::
    :maxdepth: 2
    :caption: Configurable Operators

    cat_labeling
    cat_selection
    cat_flagging
    cat_calibration
    cat_finalize

.. toctree::
    :maxdepth: 2
    :caption: Non-Configurable Operators

    internal/cat_utility

Note: documentation for the non-configurable operators is a work in progress.
