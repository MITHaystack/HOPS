Numerical Utilities
~~~~~~~~~~~~~~~~~~~

The Math library provides various numerical utilities for basic math functions,
interpolation, simple matrix operations, fringe rotation and grid calculations.
Many of these utilities have been ported from the original HOPS3 C code with minimal 
modification.

:hops:`MHO_MathUtilities`
-------------------------

=============================================== ====================================================================
File                                            MHO_MathUtilities.hh
Category                                        Numerical Utilities
Primary Functionality                           Collection of mathematical utility functions
Key Features                                    | Value clamping and boundary checking
                                                | Parabola fitting and interpolation
                                                | 3x3 matrix inversion
                                                | Linear interpolation
                                                | Angular averaging and phase calculations
                                                | Frequency limit calculations
                                                | Interval intersection finding
=============================================== ====================================================================

The MHO_MathUtilities.hh header provides a comprehensive collection of 
mathematical utility functions ported from the original HOPS3 C code. Key functions include:

- `dwin()` - Value clamping between bounds
- `parabola()` - Parabola parameter calculation and peak finding
- `minvert3()` - 3x3 matrix inversion
- `linterp()` - Linear interpolation
- `ap_mean()` - Average phase calculation
- `angular_average()` - Angular averaging in radians
- `DetermineChannelFrequencyLimits()` - Frequency range calculations
- `FindIntersection()` - Interval overlap detection

:hops:`MHO_UniformGridPointsCalculator`
---------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_UniformGridPointsCalculator`
Category                                        Numerical Utilities
Configuration Parameters                        Epsilon tolerance for point alignment
Primary Functionality                           Uniform grid calculation for frequency spacing alignment
Key Features                                    | Re-implements HOPS3 freq_spacing functionality
                                                | Creates uniform grids aligned with original points
                                                | Handles point uniqueness with epsilon tolerance
                                                | Maps original indices to uniform grid indices
                                                | Calculates grid statistics and error detection
=============================================== ====================================================================

The :hops:`MHO_UniformGridPointsCalculator` class re-implements the 
`freq_spacing` function from HOPS3. It creates uniformly spaced grids that align
with original floating-point frequency locations within specified tolerance. This 
grid calculation is required by the mult-band delay search algorithm in order to 
properly fill the visibility data into the appropriate slots for the FFT.


:hops:`MHO_FringeRotation`
--------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FringeRotation`
Category                                        Numerical Utilities
Configuration Parameters                        | SBD separation, SBD max, SBD max bin
Primary Functionality                           Fringe rotation functionality for VLBI data processing
Key Features                                    | Originally ported from HOPS3 vrot.c
                                                | Calculates fringe rotation for delay, delay-rate, time, and frequency
                                                | Sideband correction calculations
=============================================== ====================================================================

The :hops:`MHO_FringeRotation` class implements fringe rotation functionality 
needed in order to apply the VLBI fringe solution to the visibility data.
It provides the same functionality as the original HOPS3 `vrot()` function for 
calculating the fringe rotation based on delay, delay-rate, time, and frequency
parameters.



:hops:`MHO_CheckForNaN`
-----------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_CheckForNaN`
Category                                        Numerical Utilities
Template Parameters                             XNumericalType (numerical type to check)
Primary Functionality                           NaN detection for various numerical types
Key Features                                    | Generic NaN detection using value != value comparison
                                                | Specialized implementations for floating-point types
                                                | Complex number NaN detection (real and imaginary parts)
                                                | Support for float, double, and long double types
=============================================== ====================================================================

The :hops:`MHO_CheckForNaN` class provides a functor for NaN (Not-a-Number) 
detection for various numerical types. It includes:

**Supported Types:**
- `float`, `double`, `long double` - Using `std::isnan()` for consistency
- `std::complex<float>`, `std::complex<double>`, `std::complex<long double>` - Checking both real and imaginary components
- Other generic numerical types - Using `value != value` comparison

**Usage:**
The class provides a static `isnan()` method that can be used with any supported 
numerical type to detect NaN values. This method can be applied as a functor and 
broadcast across a table container/array in order to excise erroneous data.
