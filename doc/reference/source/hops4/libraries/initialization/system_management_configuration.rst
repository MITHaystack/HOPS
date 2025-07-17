System Management and Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The system management and configuration components provide the infrastructure 
for managing operator builders and parameter configuration before fringe fitting 
can take place.

:hops:`MHO_OperatorBuilder`
---------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_OperatorBuilder`
Primary Functionality                           Abstract base class for builder objects
Key Features                                    | Abstract base class with pure virtual Build() method
                                                | Manages toolbox, fringe data, container store, parameter store
                                                | SetToolbox(), SetFringeData(), SetParameterStore() methods
                                                | SetFormat(), SetConditions(), SetAttributes() methods
                                                | IsConfigurationOk() method for validation
=============================================== ====================================================================

The :hops:`MHO_OperatorBuilder` class provides the abstract base class for all 
operator builders in the system. It defines the common interface and functionality 
required by all builders, including management of various data stores and 
configuration parameters.

The class provides setter methods for toolbox, fringe data, parameter store, and 
container store management, as well as configuration validation through 
the virtual IsConfigurationOk() method.

:hops:`MHO_OperatorBuilderManager`
----------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_OperatorBuilderManager`
Primary Functionality                           Manages all operator builders in the system
Key Features                                    | Manages collections of operator builders
                                                | CreateDefaultBuilders() method for default registration
                                                | BuildOperatorCategory() method for category-based building
                                                | AddBuilderType() template method for adding new builders
                                                | Maps builders by name and category
=============================================== ====================================================================

The :hops:`MHO_OperatorBuilderManager` class manages the collection of all operator 
builders in the system. It provides centralized management of builder registration, 
categorization, and life-cycle management.

The manager includes template methods for adding new builder types and supports 
category-based building operations. It maintains mappings between builder names 
and categories for ordered operator construction/execution.

:hops:`MHO_ParameterConfigurator`
---------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ParameterConfigurator`
Primary Functionality                           Handles parameter configuration from control statements
Key Features                                    | Supports multiple parameter types (config, global, station, etc.)
                                                | Supports multiple value types (int, real, bool, string, lists)
                                                | SetScalarParameter(), GetScalarParameter() methods
                                                | SetVectorParameter(), GetVectorParameter() methods
                                                | SetCompoundParameter() method for complex parameters
=============================================== ====================================================================

The :hops:`MHO_ParameterConfigurator` class handles parameter configuration from 
control file statements. It supports all of the parameter types encountered in 
control file statements for general configuration, global, station, baseline, 
fit, and plot parameters.

The parameter configurator provides template methods for both scalar and vector parameter 
handling and supports multiple value types including integers, reals, booleans, 
strings, lists, and compound parameters.

:hops:`MHO_ParameterManager`
----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ParameterManager`
Primary Functionality                           Manager class for MHO_ParameterConfigurator
Key Features                                    | Manages MHO_ParameterConfigurator instances
                                                | SetControlStatements() method for control file input
                                                | ConfigureAll() method for processing all parameters
                                                | Coordinates parameter configuration across the system
=============================================== ====================================================================

The :hops:`MHO_ParameterManager` class serves as the manager for MHO_ParameterConfigurator 
instances. It handles control file input and processes all control file parameters systematically.
The manager provides methods for setting control statements and triggering full configuration of 
all parameters through the ConfigureAll() method.
