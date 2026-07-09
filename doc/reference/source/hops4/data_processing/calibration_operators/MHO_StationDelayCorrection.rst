MHO\_StationDelayCorrection
=============================

Purpose
-------

This operator applies a station-specific delay correction to visibility data by
phase-rotating each visibility according to the frequency offset from the
reference frequency. It is used to compensate for timing offsets
(e.g., PC-clock delay) between the two stations in a baseline, typically arising
from hardware or clock synchronization differences.

Control File Trigger
--------------------

This operator is triggered by the ``station_delay`` keyword in the control file.
It belongs to the calibration category with priority 3.5. The parameter is a
real value representing the delay offset in nanoseconds.

Input Data
----------

The operator works in-place on a ``visibility_type`` container. The visibility
must carry metadata tags identifying the reference and remote stations
(Mark4 IDs or station codes) so the operator can determine which station the
correction applies to. The container axes are:

- **POLPROD\_AXIS** -- polarization product axis
- **CHANNEL\_AXIS** -- spectral channel axis (labeled by center frequency in MHz, with ``net_sideband`` key per channel)
- **TIME\_AXIS** -- accumulation period axis
- **FREQ\_AXIS** -- intra-channel frequency bin axis

Algorithm
---------

The operator loops over the two stations (reference at index 0, remote at index 1) and checks applicability for each via ``IsApplicable()``. Station matching supports both single-character Mark4 IDs and two-character station codes (case-insensitive). A wildcard identifier (``?`` for Mark4 ID, ``??`` for station code) matches any station.

For an applicable station, the operator iterates over every polarization product and channel, computing a phase-correction phasor. The core equation is:

.. math::

   \theta = 2\pi \cdot \Delta f \cdot \Delta t

.. math::

   \Delta f = 10^6 \cdot (f_{\rm chan} - f_{\rm ref})

.. math::

   \Delta t = \tau_{\rm ns} \cdot 10^{-9}

.. math::

   p = e^{j\theta}

where :math:`f_{\rm chan}` is the channel center frequency in MHz, :math:`f_{\rm ref}` is the reference frequency in MHz, :math:`\tau_{\rm ns}` is the delay offset in nanoseconds, and :math:`j` is the imaginary unit.

The phasor is conjugated under two conditions:

1. **Lower sideband (LSB)** data: the phasor is conjugated because LSB has inverted frequency ordering relative to the IF.
2. **Reference station** (index 0): the phasor is conjugated because the reference station's delay correction has the opposite sign convention.

Note that if both conditions apply (reference station + LSB), the two conjugations cancel, yielding the original phasor.

Effect on Data
--------------

This operator modifies the input visibility container in-place. Each visibility
sample for the matched station is multiplied by a complex
phasor :math:`e^{j\theta}` (or its conjugate, depending on
sideband and station role). The phase rotation shifts the
apparent delay of the signal, effectively compensating for a station
clock or hardware delay offset. The axis labels and metadata tags are unchanged.
