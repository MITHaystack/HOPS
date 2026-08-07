MHO_InterpolateFringePeakOptimized
===================================

Purpose
-------
Implements the fine peak interpolation phase of the wideband fringe-fitting
pipeline. After the coarse 3D search (delay rate, multi-band delay, single-band delay)
identifies the best integer bin, this operator refines the peak location
to sub-bin precision by counter-rotating normalized visibilities on
a $5 \times 5 \times 5$ grid about the coarse maximum, then performing
iterative 5-point Lagrange interpolation on an $11 \times 11 \times 11$ grid.

Control File Trigger
--------------------
This operator is internal and not directly triggerable from the control file.
It is used by the ``MHO_FringeFitter`` as step 3 of the fringe-fitting pipeline.
The ``optimize_closure`` fit parameter can enable closure optimization during
counter-rotation.

.. list-table:: Configuration parameters
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - ``optimize_closure``
     - bool
     - Enable closure optimization (not used for ``simul`` method)

Input Data
----------
This operator does not inherit from ``MHO_UnaryOperator`` or ``MHO_BinaryOperator``.
Instead, it takes explicit inputs set via setter methods:

- ``visibility_type`` pointer (SBD-transformed visibility data)
- ``weight_type`` pointer (associated weights)
- ``time_axis_type`` (MBD axis)
- ``delay_rate_axis_type`` (DR axis)
- Integer bin coordinates ``(sbd_max, mbd_max, dr_max)`` from coarse search
- Reference frequency and fourfit reference time offset

Algorithm
---------
The algorithm follows the original ``interp.c`` (SIMUL mode) logic, with
significant optimizations to avoid redundant ``exp()`` calls.

**Step 1: Precompute phasors**

Two categories of phasors are precomputed outside the main loop:

- **Channel phasor** ``channel_phasor[imbd][fr]`` for each of 5 MBD offsets and each channel:

  .. math::
     P_{\mathrm{ch}} = \exp\left(-2\pi i \left[\mathrm{mbd} \cdot (f_{\mathrm{ch}} - f_{\mathrm{ref}}) + \mathrm{sb\_corr}(\mathrm{mbd})\right]\right)

- **Delay-rate step phasor** ``step_phasor[idr][fr]`` for AP-to-AP recurrence:

  .. math::
     P_{\mathrm{step}} = \exp\left(-2\pi i \cdot f_{\mathrm{ch}} \cdot \mathrm{dr} \cdot \Delta t_{\mathrm{ap}}\right)

- **AP-0 phasor** ``ap0_phasor[idr][fr]`` for the first accumulation period:

  .. math::
     P_{\mathrm{ap0}} = \exp\left(-2\pi i \cdot f_{\mathrm{ch}} \cdot \mathrm{dr} \cdot \Delta t_0\right)

**Step 2: Fill $5 \times 5 \times 5$ cube**

For each of the 125 trial points, the operator counter-rotates all visibilities from all channels and APs, then sums coherently:

.. math::
   Z = \frac{1}{W_{\mathrm{tot}}} \sum_{\mathrm{ch}} \sum_{\mathrm{ap}} V_{\mathrm{ch,ap}} \cdot P_{\mathrm{ch}} \cdot P_{\mathrm{ap}} \cdot w_{\mathrm{ch,ap}}

The AP phasor advances by multiplication (``phasor *= step``) rather than recomputing ``exp()``, reducing the inner loop to 2 multiplications per spectral sample. The cube value is the magnitude: ``drf[isbd][imbd][idr] = |Z|``.

**Step 3: Iterative peak refinement (``max555``)**

Starting from the center of the cube, the algorithm iteratively:

1. Compress the search range to fit within bounds (SBD: $\pm 1$ bin, MBD/DR: $\pm 2$ bins)
2. Evaluate the interpolated function on an $11 \times 11 \times 11$ grid
3. Find the new maximum
4. Relocate center to new maximum and reduce grid spacing by factor of 5
5. Converge when grid spacing $< 10^{-4}$

**Step 4: 5-point Lagrange interpolation (``interp555``)**

Trilinear 5-point Lagrange interpolation using Abramowitz & Stegun 25.2.15 weights:

.. math::
   a_0 = \frac{(p^2-1)p(p-2)}{24}, \quad
   a_1 = -\frac{(p-1)p(p^2-4)}{6}, \quad
   a_2 = \frac{(p^2-1)(p^2-4)}{4}, \quad
   a_3 = -\frac{(p+1)p(p^2-4)}{6}, \quad
   a_4 = \frac{(p^2-1)p(p+2)}{24}

Effect on Data
--------------
This operator does not modify the input data. It produces scalar output
quantities accessible via getter methods:

- ``fSBDelay`` -- refined single-band delay (ns)
- ``fMBDelay`` -- refined multi-band delay (ns)
- ``fDelayRate`` -- refined delay rate (ns/s)
- ``fFringeRate`` -- fringe rate (Hz)
- ``fFringeAmp`` -- fringe amplitude (normalized)
