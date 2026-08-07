MHO\_DelayModel
===============

Purpose
-------
``MHO_DelayModel`` evaluates the station a priori delay-model polynomials for
both the reference and remote stations at a given fourfit reference time. It
computes the baseline delay, delay rate, and acceleration as the difference
between the remote and reference station spline evaluations. Additionally, it
calculates reference-station-specific delay and rate quantities (used for
populating type-208 records on Mark4 export).

Control File Trigger
--------------------
This operator is internal and not directly triggerable from the control file.
It is instantiated and used by the fringe-fitting core to obtain a priori
delay-model values at the fourfit reference time.

Input Data
----------
This class operates on ``station_coord_type`` containers for both the reference
and remote stations. These containers hold delay-model spline coefficients along
with metadata such as model start time, model interval, and station identification.

Algorithm
---------
``MHO_DelayModel`` has no ``Initialize`` method; all work occurs in ``ComputeModel``.

**Model Computation (``ComputeModel``):**

1. Convert the fourfit reference time from its VEX-format string to a ``hops_clock`` time point.
2. Retrieve the station codes (``station_code``) for both reference and remote stations from their coordinate containers.
3. Retrieve and convert the delay-model start times (``model_start``) and model intervals (``model_interval``) for both stations.
4. Calculate the time difference between the fourfit reference time and each station's model start time, expressed in seconds.
5. Determine the spline interval index for each station: ``int = floor(t_diff / t_interval)``. Clamp the index to ``[0, N_intervals - 1]`` and warn if extrapolation is required.
6. Calculate the offset within the selected interval: ``t = t_diff - int * t_interval``.
7. Extract the spline coefficient sub-vector for the selected interval from each station's container.
8. Evaluate the delay spline polynomial for each station using the coefficients and time offset. For a spline with coefficients :math:`c_0, c_1, \dots, c_{N-1}` and time offset :math:`\Delta t`:

   .. math::

      \text{delay} = \sum_{p=0}^{N-1} c_p \, \Delta t^p

      \text{rate} = \sum_{p=1}^{N-1} p \, c_p \, \Delta t^{p-1}

      \text{accel} = \sum_{p=2}^{N-1} p(p-1) \, c_p \, \Delta t^{p-2}

9. Compute the baseline delay, rate, and acceleration as the difference between the remote and reference station values.
10. For reference-station export quantities (Mark4 I/O), correct the reference delay for clock offset and rate, adjust for the approximate time of wavefront passage, and compute the Doppler factor ``ref_doppler = 1 - rate_ref``. Re-evaluate both station splines at the adjusted time (shifted by the reference delay), then compute ``fRefDelay``, ``fRefRate`` (scaled by Doppler), and ``fRefStationDelay``.

Effect on Data
--------------
This class does not modify any container data. It reads spline coefficients
from ``station_coord_type`` containers and computes delay, rate, and
acceleration values stored internally. The results are accessible
via ``GetDelay()``, ``GetRate()``, and ``GetAcceleration()`` getters.
