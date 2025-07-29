CUDA Acceleration
~~~~~~~~~~~~~~~~~

The CUDA plugin provides GPU-acceleration for computationally intensive 
operations during VLBI fringe fitting. At the moment this is primarily focusing on the (multi-band) 
delay search algorithm using NVIDIA's CUDA framework and cuFFT library. 

**This plugin is experimental and not recommended for production use.**

:hops:`MHO_MBDelaySearchCUDA`
-----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_MBDelaySearchCUDA`
Primary Functionality                           GPU-accelerated multi-band delay search implementation
Key Features                                    | Inherits from MHO_MBDelaySearch for compatibility
                                                | Uses NVIDIA cuFFT library for FFT acceleration
                                                | Host and device memory buffer management
                                                | CUDA runtime API integration
                                                | Accelerates inner-most loop over delay-rate/MBD space
=============================================== ====================================================================

The :hops:`MHO_MBDelaySearchCUDA` class provides a basic CUDA implementation of the 
coarse multi-band delay (MBD) search algorithm. This implementation uses NVIDIA's 
cuFFT library to accelerate the  computationally intensive FFT operations required for delay search.

**Implementation Details:**
    - Uses CUDA runtime API for device memory management
    - Integrates cuFFT for device FFT operations
    - Supports cuComplex data types for complex number operations
    - Requires CUDA-capable GPU hardware for execution

**Performance Notes:**
The current implementation ultra basic and very primitive with significant optimization remaining to be done.
The main current limitation is excessive data movement between host and device memory, which will be improved in future versions.

**Usage Requirements:**
    - NVIDIA GPU with CUDA Compute Capability 2.0 or higher
    - CUDA Runtime and cuFFT libraries installed
    - Appropriately sized GPU memory for processing large datasets
