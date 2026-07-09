Selection Operators
===================

Selection operators restrict which data enter the fringe fit based on quality
thresholds. They run after labeling but before flagging.

In addition to the user-configurable ``min_weight`` threshold below, an internal
coarse-selection/repacking step always runs in this category, independent of any
control-file statement.

.. toctree::
    :maxdepth: 1

    MHO_MinWeight
