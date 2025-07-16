..  _Calibration:

Calibration
===========

The calibration library contains a collection of operators which can manipulate 
the visibility data in various ways. These operators are organized by category 
based on their functionality. These are broadly split into two groups, configurable
and non-configurable operators. Operators that are configurable are those which 
can be triggered, or which have parameters that are accessible through user issued 
control file statements or ``fourfit`` command line arguments. While those which
are non-configurable either compartmentalize a portion of the underlying 
fringe-finding algorithm, or are triggered by a specific data attribute.

Of the configurable operators, the available categories are labeling, selection, 
flagging, calibration, and finalization (finalize). There are also 'pre-fit' and
'post-fit' operator categories which are available for user python plugins, that
execute before and after the fringe-peak search routine. During runtime each 
category is separately initialized and executed on the data in the following order:

    #. labeling 
    #. selection 
    #. flagging 
    #. calibration
    #. pre-fit 
    #. post-fit 
    #. finalize

The calibration library also contains some additional utility classes 
(e.g. MHO_StationModel, MHO_DelayModel) for calculating quantities needed during 
fringe fitting.

.. toctree::
    :maxdepth: 2
    :caption: Calibration Operators by Category

    labeling
    flagging
    calibration
    selection
    finalize
    non_configurable
    non_operator
