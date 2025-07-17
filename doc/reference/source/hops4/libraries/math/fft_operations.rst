Fast Fourier Transform Operations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The HOPS4 math library provides a native Fast Fourier Transform (FFT) 
implementation which supports both radix-2 and arbitrary-length transforms. 
These base routines will not be as performant as FFTW3, but may be used on platforms 
where the FFTW3 library is not available. These FFT operations follow FFTW3 
conventions (e.g. forward/backward sign conventions) and include workspace 
management and bit manipulation utilities.

:hops:`MHO_FastFourierTransformCalls`
-------------------------------------

=============================================== ====================================================================
File                                            MHO_FastFourierTransformCalls.hh
Category                                        FFT Operations
Template Parameters                             XFloatType (floating-point type: float, double, long double)
Primary Functionality                           Native FFT implementation calls for radix-2 and Bluestein algorithms
Key Features                                    | Radix-2 Decimation-in-Time (DIT) FFT algorithm
                                                | Bluestein's FFT algorithm for arbitrary lengths
                                                | Template support for different floating-point precisions
                                                | Workspace-based FFT computation
=============================================== ====================================================================

The MHO_FastFourierTransformCalls.hh header provides the main FFT computation functions. 
It includes `FFTRadix2()` for power-of-two length transforms using the 
Radix-2 DIT algorithm, and `FFTBluestein()` for arbitrary-length transforms 
using Bluestein's algorithm.

:hops:`MHO_FastFourierTransformUtilities`
-----------------------------------------

=============================================== ====================================================================
File                                            MHO_FastFourierTransformUtilities.hh
Category                                        FFT Operations
Template Parameters                             XFloatType (floating-point type: float, double, long double)
Primary Functionality                           Basic utility functions for native FFT implementations
Key Features                                    | Twiddle factor computation and conjugation
                                                | Radix-2 DIT and DIF FFT algorithms
                                                | Cooley-Tukey and Gentleman-Sande butterfly operations
                                                | Bluestein algorithm utilities (scale factors, circulant vectors)
=============================================== ====================================================================

The MHO_FastFourierTransformUtilities.hh header provides basic FFT 
building blocks including the twiddle factor computation, butterfly operations, 
and other support functions needed by both the radix-2 and Bluestein algorithms.

:hops:`MHO_FastFourierTransformWorkspace`
-----------------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_FastFourierTransformWorkspace`
Category                                        FFT Operations
Template Parameters                             XFloatType (floating-point type: float, double, long double)
Configuration Parameters                        Transform size N
Primary Functionality                           Workspace management for FFT operations
Key Features                                    | Automatic workspace allocation and memory management
                                                | Support for both power-of-2 and arbitrary-length transforms
                                                | Radix-2 detection and optimization
                                                | Twiddle factor and permutation storage
=============================================== ====================================================================

The :hops:`MHO_FastFourierTransformWorkspace` class manages workspace allocation
and data structures required for FFT computations. 
It automatically determines whether to use radix-2 or Bluestein algorithms based
on the transform size.

:hops:`MHO_BitReversalPermutation`
----------------------------------

=============================================== ====================================================================
File                                            MHO_BitReversalPermutation.hh
Category                                        FFT Operations
Template Parameters                             DataType (for array permutation functions)
Configuration Parameters                        None
Primary Functionality                           Bit manipulation utilities for power-of-two FFTs
Key Features                                    | Power-of-two detection and calculations
                                                | Bit reversal permutation algorithms
                                                | Buneman algorithm for bit-reversed indices
                                                | Branch-free array permutation
=============================================== ====================================================================

The MHO_BitReversalPermutation.hh header provides bit manipulation utilities 
essential for radix-2 FFT algorithms, including power-of-two detection, 
bit reversal operations, and array permutation functions.
