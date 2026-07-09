Data Selection and Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The data selection and filtering components provide builders for operators that
control which data is included in the fringe fitting process based on various
selection criteria and quality metrics.

:hops:`MHO_DataSelectionBuilder`
--------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_DataSelectionBuilder`
Primary Functionality                           Builds a data selection operator
Key Features                                    | Constructs and initializes data selection operator
                                                | Acts on fringe data for selection operations
                                                | Inherits from MHO_OperatorBuilder
=============================================== ====================================================================

The :hops:`MHO_DataSelectionBuilder` class builds a data selection operator that
controls which data is included in the fringe fitting process. This operator can
filter data based on AP-offset time windows, named frequency groups, specific
channel labels, and/or polarization-product sets. Selection by minute-past-the-hour
(positive `start`/`stop` values, per the legacy fourfit convention) is not yet
supported; only negative `start`/`stop` values (seconds relative to scan start/stop)
are currently honored, and positive values are rejected with an error and no
selection is applied.

The builder provides flexible data selection capabilities for fringe fitting
operations.

:hops:`MHO_MinWeightBuilder`
----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_MinWeightBuilder`
Primary Functionality                           Builds MHO_MinWeight operator for weight-based filtering
Key Features                                    | Constructs and adds MHO_MinWeight operator to toolbox
                                                | Cuts data with weight less than threshold
                                                | Implements quality-based data filtering
=============================================== ====================================================================

The :hops:`MHO_MinWeightBuilder` class builds a minimum weight operator that filters
data based on weight thresholds. This operator removes data points that have weights
below a specified minimum value, effectively filtering out low-quality or unreliable
data from the fringe fitting process. If the configured threshold is `0`, the
operator is not constructed at all (rather than applying a no-op threshold), so no
weight-based filtering occurs.
