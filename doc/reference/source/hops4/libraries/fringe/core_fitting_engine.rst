Core Fringe Fitting Engine
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The core fringe fitting engine provides the main interface through which VLBI 
fringe fitting is executed. This library provides the abstract base
class MHO_FringeFitter which provides the main interface, as well
as concrete implementations for single-baseline fringe fitting with or with 
ionospheric dTEC correction.

:hops:`MHO_FringeFitter`
------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringeFitter`
Primary Functionality                           Abstract base class for fringe fitter implementation
Key Features                                    | Pure virtual interface for fringe fitting workflow
                                                | Data store management (parameters, scan data, containers)
                                                | Visitor pattern support for extensibility
                                                | Operator toolbox integration
=============================================== ====================================================================

The :hops:`MHO_FringeFitter` class provides the abstract base class interface for all 
fringe fitters in the HOPS4 framework. It defines the core workflow methods that must 
be implemented by concrete fitter classes: Configure(), Initialize(), Run(), and Finalize().
The class manages access to various data stores including parameter stores, scan data, 
and container objects needed for fringe fitting operations.

:hops:`MHO_BasicFringeFitter`
-----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_BasicFringeFitter`
Primary Functionality                           Basic single-baseline single-polarization-product fringe fitter
Key Features                                    | Implements basic workflow: configure, init, run, finalize
                                                | Mixed/single sideband detection and processing
                                                | Coarse fringe search in delay/delay-rate space
                                                | Peak interpolation over 5x5x5 grid
                                                | Visitor pattern support
=============================================== ====================================================================

The :hops:`MHO_BasicFringeFitter` class implements the basic fringe fitting algorithm 
for single-baseline, single-polarization product data. It provides the core functionality 
for delay and delay-rate space search. Implemented as a coarse fringe search (max bin), 
followed by a fine peak interpolation step. This fitter includes support for 
both mixed and single sideband channel processing modes.

:hops:`MHO_IonosphericFringeFitter`
-----------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_IonosphericFringeFitter`
Primary Functionality                           Single-baseline fringe fitter with ionosphere search
Key Features                                    | Extends MHO_BasicFringeFitter functionality
                                                | dTEC (differential Total Electron Content) search
                                                | Optional smoothing function support during search
                                                | Ionospheric covariance calculations
=============================================== ====================================================================

The :hops:`MHO_IonosphericFringeFitter` class extends the basic fringe fitter to include 
ionospheric dispersion correction capabilities. It performs searches in dTEC/delay/delay-rate 
space and includes an optional smoothing function step for more robust ionospheric parameter estimation.
This fitter is essential for high-precision VLBI observations where ionospheric effects 
must be properly modeled and corrected. This implementation is a close port of R. Cappalo's original implementation
but has been extended to allow for an arbitrary number of points in the dTEC search space.

:hops:`MHO_FringeFitterFactory`
-------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringeFitterFactory`
Primary Functionality                           Factory for constructing appropriate fringe fitter types
Key Features                                    | Factory pattern implementation
                                                | Support for different fitter types (Basic, Ionospheric)
                                                | Extensible for future fitter implementations
=============================================== ====================================================================

The :hops:`MHO_FringeFitterFactory` class implements the factory pattern for constructing 
the appropriate fringe fitter based on the user's configuration parameters. 
It provides a simple mechanism for creating Basic or Ionospheric fitters as 
needed by the processing pipeline. This design pattern allows for easy 
extension to support additional fitter types in the future without modification of
any of the pre-existing fringe fitter implementations.
