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
                                                | Uses MHO_Tokenizer for processing
                                                | Inherits from MHO_OperatorBuilder
=============================================== ====================================================================

The :hops:`MHO_DataSelectionBuilder` class builds a data selection operator that 
controls which data is included in the fringe fitting process. This operator can 
apply various selection criteria to filter data based on time ranges, frequency 
ranges, polarization products, or other data characteristics.

The builder uses the MHO_Tokenizer for processing selection criteria and provides 
flexible data selection capabilities for fringe fitting operations.

:hops:`MHO_MinWeightBuilder`
----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_MinWeightBuilder`
Primary Functionality                           Builds MHO_MinWeight operator for weight-based filtering
Key Features                                    | Constructs and adds MHO_MinWeight operator to toolbox
                                                | Cuts data with weight less than threshold
                                                | Implements quality-based data filtering
                                                | Returns bool indicating successful construction
=============================================== ====================================================================

The :hops:`MHO_MinWeightBuilder` class builds a minimum weight operator that filters 
data based on weight thresholds. This operator removes data points that have weights 
below a specified minimum value, effectively filtering out low-quality or unreliable 
data from the fringe fitting process.

The builder provides boolean feedback on the success of the operator construction 
and is essential for maintaining data quality in VLBI processing pipelines.
