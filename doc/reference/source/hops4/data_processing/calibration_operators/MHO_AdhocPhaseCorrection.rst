MHO\_AdhocPhaseCorrection
=========================

Purpose
-------
``MHO_AdhocPhaseCorrection`` applies a time-dependent (and optionally
channel-dependent) phase correction ``exp(-i*zeta)`` to visibility data.
The operator supports three modes: sinewave, polynomial, and file-based.
The phase correction ``zeta`` is computed per (channel, accumulation period)
and applied uniformly across all polarization products and spectral points.

Control File Trigger
--------------------
- **Keyword:** ``adhoc_phase``
- **Category:** calibration
- **Priority:** 3.5

.. list-table:: Parameters for ``adhoc_phase`` (sinewave mode)
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - algorithm_type
     - string
     - Must be ``sinewave``.
   * - adhoc_amp
     - double
     - Sinewave amplitude in degrees.
   * - adhoc_period
     - double
     - Sinewave period in seconds.
   * - adhoc_tref
     - double
     - Reference time in seconds past the most recent hour.

.. list-table:: Parameters for ``adhoc_phase`` (polynomial mode)
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - algorithm_type
     - string
     - Must be ``polynomial``.
   * - adhoc_poly
     - list_real
     - 1-6 polynomial coefficients in deg/s^n.
   * - adhoc_tref
     - double
     - Reference time in seconds past the most recent hour.

.. list-table:: Parameters for ``adhoc_phase`` (file mode)
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - algorithm_type
     - string
     - Must be ``file``.
   * - adhoc_file
     - string
     - Per-station: path to the ASCII phase file.
   * - adhoc_file_chans
     - string
     - Per-station: channel-label string defining column order.

Input Data
----------
This operator acts on the ``visibility_type`` container.

Algorithm
---------
The operator performs the following steps:

**Initialization (``InitializeInPlace``):**

1. If the mode is ``NONE``, the operator is a no-op.
2. Extract the scan start time from the visibility container's ``start`` metadata tag. Convert to fractional days since the beginning of the year, and compute the scan start in seconds past the most recent hour.
3. Derive the accumulation period duration from the time axis.
4. For file (PHYLE) mode only: load and parse both the reference and remote station adhoc phase files. Each data line contains a fractional day time followed by one phase value (in degrees) per channel. Comment lines (non-numeric first token) are skipped. If a file has only one data row, it is duplicated to ensure a valid interpolation interval.

**Execution (``ExecuteInPlace``):**

1. Iterate over every frequency channel in the visibility container.
2. For each channel, retrieve the ``channel_label`` (fourfit freq-code character). Channels without a label are skipped.
3. For each accumulation period (AP):

   a. Compute the AP center time in seconds from scan start: ``t_center = t_AP + 0.5 * t_acc``.
   b. Compute the phase correction ``zeta`` (in radians) by calling ``ComputeZeta`` with the channel label and AP center time (details below).
   c. Construct the phasor ``Phi = exp(-i*zeta)``.
   d. Multiply all visibility values for all polarization products and spectral points at (channel, AP) by ``Phi``.

**Phase Computation (``ComputeZeta``):**

The time argument used in the sinewave and polynomial models is:

.. math::

   \tau = t_{\rm center} + t_{\rm scan\_start\_fpday} \cdot 86400 - t_{\rm ref}

where :math:`t_{\rm ref}` is the ``adhoc_tref`` parameter. This (legacy) convention
measures time in seconds since the beginning of the year, minus the seconds-past-the-hour reference.

- **Sinewave mode:**

  .. math::

     \zeta = A \sin\!\left(\frac{2\pi \tau}{P}\right)

  where :math:`A` is the amplitude (converted from degrees to radians by the builder) and :math:`P` is the period in seconds.

- **Polynomial mode:**

  .. math::

     \zeta = c_0 + c_1 \tau + c_2 \tau^2 + c_3 \tau^3 + c_4 \tau^4 + c_5 \tau^5

  where coefficients :math:`c_0, \dots, c_5` are in rad/s\ :sup:`n` (converted from degrees by the builder). Up to 6 coefficients are used; missing entries default to zero.

- **File mode:**

  1. Convert the AP center time to fractional days since BOY: ``t_fpday = t_scan_start_fpday + t_center / 86400``.
  2. For each station, look up the channel's freq-code character in the station's channel string to find the column index.
  3. Clamp ``t_fpday`` to the file's time range, then find the bounding interval [n-1, n] by linear scan.
  4. Linearly interpolate the phase (in degrees) between rows n-1 and n:

     .. math::

        \phi_{\rm deg} = \frac{t_{\rm bound}(\phi_b - \phi_a) - t_a \phi_b + t_b \phi_a}{t_b - t_a}

  5. Convert from degrees to radians.
  6. The differential phase correction is :math:`\zeta = \phi_{\rm ref} - \phi_{\rm rem}`.


Effect on Data
--------------
For each (channel, AP) combination, all visibility values across all
polarization products and spectral points are multiplied by a complex
phasor ``exp(-i*zeta)``, where ``zeta`` is the mode-dependent phase correction
in radians. In sinewave and polynomial modes, ``zeta`` is the same for all
channels. In file mode, ``zeta`` varies per channel and is computed as the
differential phase (reference minus remote) interpolated from station-specific 
phase files.
