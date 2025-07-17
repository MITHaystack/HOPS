Visualization and Output
~~~~~~~~~~~~~~~~~~~~~~~~

The visualization and output components provide functionality for generating 
fringe plots and managing plot data using the visitor pattern for extensible 
plotting capabilities.

:hops:`MHO_FringePlotInfo`
--------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringePlotInfo`
Primary Functionality                           Wrapper around plot data construction utilities
Key Features                                    | Plot data construction from various data stores
                                                | VEX information integration
                                                | JSON-based plot data storage
                                                | Comprehensive data organization for plotting
=============================================== ====================================================================

The :hops:`MHO_FringePlotInfo` class provides a wrapper around variouis plot data construction 
utilities. It constructs plot data from various data stores and integrates VEX 
information. It is intended to decouple the computation of fringe data from the plot mechanism.
Key functions include `construct_plot_data()` for building plot data from stores
and `fill_plot_data()` for populating JSON objects with plot information.

:hops:`MHO_FringePlotVisitor`
-----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringePlotVisitor`
Primary Functionality                           Visitor pattern implementation for fringe plotting
Key Features                                    | Inherits from MHO_FringeFitterVisitor
                                                | Provides extensible plotting framework
                                                | Pure virtual plotting interface
                                                | Allows for multiple plotting implementations
=============================================== ====================================================================

The :hops:`MHO_FringePlotVisitor` class implements the visitor pattern for fringe 
plotting operations. It provides an extensible framework that allows different 
plotting implementations to be used with the same fringe fitting infrastructure.
The class includes a pure virtual `Plot()` method that must be implemented by 
concrete visitor classes. 

:hops:`MHO_DefaultPythonPlotVisitor`
------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_DefaultPythonPlotVisitor`
Primary Functionality                           Default Python plotting implementation
Key Features                                    | Inherits from MHO_FringePlotVisitor
                                                | Provides default Python plotting behavior
                                                | Integrates with Python plotting utilities
                                                | Concrete implementation of visitor pattern
=============================================== ====================================================================

The :hops:`MHO_DefaultPythonPlotVisitor` class provides a concrete implementation 
of the fringe plot visitor pattern using Python plotting utilities (matplotlib). It serves as 
the default plotting implementation for the HOPS4 fringe fitting framework. 
At the moment, it is the only plotting implementation available in HOPS4, but future 
extensions are planned.
