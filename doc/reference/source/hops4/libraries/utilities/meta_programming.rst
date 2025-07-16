Meta-Programming Utilities
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The utilities library provides several template meta-programming tools for 
type manipulation and compile-time computations.

MHO_Meta
--------

=============================================== ====================================================================
File                                            MHO_Meta.hh
Category                                        Meta-Programming
Template Parameters                             Variadic template parameters for type manipulation
Primary Functionality                           Template meta-programming utilities and type manipulation
Key Features                                    | Tuple access and modification utilities
                                                | Type counting and identification
                                                | Runtime-indexed tuple access
                                                | Complex type detection and switching
=============================================== ====================================================================

The MHO_Meta.hh header provides template meta-programming tools, such as tuple manipulation, 
type counting, runtime-indexed tuple access, and type detection capabilities.

:hops:`MHO_ClassIdentity`
-------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_ClassIdentity`
Category                                        Meta-Programming
Template Parameters                             Template methods for class type identification
Primary Functionality                           Runtime class name extraction and UUID generation
Key Features                                    | Template-based class name deduction
                                                | MD5-based UUID generation from class names
                                                | Type identification for serialization
                                                | Class version error reporting
=============================================== ====================================================================

The :hops:`MHO_ClassIdentity` class provides runtime class name extraction and UUID generation for 
type identification in serialization and file operations.

MHO_TemplateTypenameDeduction
-----------------------------

=============================================== ====================================================================
File                                            MHO_TemplateTypenameDeduction.hh
Category                                        Meta-Programming
Template Parameters                             Template functions for type name extraction
Configuration Parameters                        Compiler-specific (Clang/GCC support)
Primary Functionality                           Compile-time type name extraction with runtime processing
Key Features                                    | Cross-compiler type name deduction
                                                | String cleanup for readable class names
                                                | Special handling for std::string types
                                                | Namespace prefix removal
=============================================== ====================================================================

The MHO_TemplateTypenameDeduction.hh header provides compile-time type 
name extraction with runtime string processing to generate consistent class names across 
different compilers. These class names are used for object type identification via 
MD5 hashing, which is used for file key (:hops:`MHO_FileKey`)
generation during the object serialization.

MHO_Types
---------

=============================================== ====================================================================
File                                            MHO_Types.hh
Category                                        Meta-Programming
Compile time parameters                         ENSURE_PORTABILITY flag (enabled by default)
Primary Functionality                           Type size validation for data portability
Key Features                                    | Static assertions for type size validation
                                                | Ensures consistent data layout across platforms
                                                | Prevents compilation on non-portable systems
=============================================== ====================================================================

The MHO_Types.hh header provides type size validation through static assertions 
to ensure data portability. Triggers a compile time error on non-compatible platforms/compilers.
