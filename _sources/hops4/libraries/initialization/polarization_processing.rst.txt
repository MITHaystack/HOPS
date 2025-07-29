Polarization Handling
~~~~~~~~~~~~~~~~~~~~~

The polarization handling components provide builders for operators that process 
polarization-specific corrections and transformations in VLBI fringe fitting, 
including field rotation, parallactic angle corrections, and polarization 
product summation.

:hops:`MHO_CircularFieldRotationBuilder`
----------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_CircularFieldRotationBuilder`
Primary Functionality                           Builds a MHO_CircularFieldRotationCorrection operator
Key Features                                    | Constructs and adds CircularFieldRotationBuilder to toolbox
                                                | Inherits from MHO_OperatorBuilder
                                                | Requires station mount type information
                                                | Handles field rotation corrections for circular polarizations
=============================================== ====================================================================

The :hops:`MHO_CircularFieldRotationBuilder` class builds a circular field rotation 
correction operator that compensates for field rotation effects in circular polarization 
observations. This correction is essential for maintaining proper polarization alignment 
as the Earth rotates relative to the observed source. This builder requires 
station mount type information to properly calculate the field
rotation corrections for each station in the VLBI array.

:hops:`MHO_LinearDParCorrectionBuilder`
---------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_LinearDParCorrectionBuilder`
Primary Functionality                           Builds a delta-parallactic angle correction operator
Key Features                                    | Constructs linear-pol DPar correction operator
                                                | Handles parallactic angle corrections for linear polarization
                                                | Returns bool indicating construction success
                                                | Essential for linear polarization observations
=============================================== ====================================================================

The :hops:`MHO_LinearDParCorrectionBuilder` class builds a delta-parallactic angle 
correction operator for linear polarization observations. This correction compensates 
for the effect of the parallactic angle difference in the magnitude of the power 
distributed between the parallel and cross-handed polarization products. It is 
only applied when a Pseudo-Stoke-I summation of all four pol-products (XX, XY, YY, YX) is
requested.

:hops:`MHO_MixedPolYShiftBuilder`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_MixedPolYShiftBuilder`
Primary Functionality                           Builds operator for 90-degree phase shift to Y pol in mixed fringes
Key Features                                    | Constructs MHO_MixedPolYShift operator
                                                | Handles mixed linear-circular fringe polarization corrections
                                                | Applies 90-degree phase shift to Y polarization
=============================================== ====================================================================

The :hops:`MHO_MixedPolYShiftBuilder` class builds an operator that applies a 90-degree 
phase shift to the Y polarization in mixed linear-circular fringe observations. This 
correction is a useful approximation when processing data that combines linear and circular 
polarization systems.

:hops:`MHO_PolProductSummationBuilder`
--------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PolProductSummationBuilder`
Primary Functionality                           Builds operator for polarization product summation
Key Features                                    | Constructs PolProductSummation operator
                                                | Handles polarization product summation
                                                | Supports pseudo-Stokes-I calculations
                                                | Returns bool indicating construction success
=============================================== ====================================================================

The :hops:`MHO_PolProductSummationBuilder` class builds an operator that performs 
summation over specified polarization products. This is necessary for creating 
composite polarization measurements such as pseudo-Stokes-I parameters, but also 
sums of the form RR+LL. This functionality is useful for total intensity measurements
from polarized VLBI observations.
