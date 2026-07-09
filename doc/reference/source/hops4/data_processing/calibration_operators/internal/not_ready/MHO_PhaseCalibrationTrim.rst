MHO\_PhaseCalibrationTrim
=========================

Purpose
-------
``MHO_PhaseCalibrationTrim`` trims the time axis of multitone phase
calibration (pcal) data so that it matches the time range of the visibility
data. This operator is an internal utility used by the fringe fitter to ensure
that pcal data and visibility data share the same accumulation periods before
phase correction is applied. The implementation assumes that pcal and visibility
data have the same accumulation period (AP).

Control File Trigger
--------------------
This operator is internal to the fringe-fitting pipeline and has no control
file keyword. It is instantiated programmatically by the fringe fitter and
configured with a reference to the visibility container via ``SetVisibilities``.

Input Data
----------
This operator acts on the ``multitone_pcal_type`` container. It also requires a const reference to a ``visibility_type`` container (supplied via ``SetVisibilities``) to determine the target time range.

Algorithm
---------
``MHO_PhaseCalibrationTrim`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

**Execution (``ExecuteInPlace``):**

1. Validate that both the pcal and visibility pointers are non-null.
2. Retrieve the ``start`` metadata tag from both containers to obtain scan start times in VEX time format. Also retrieve the ``station_code`` from the pcal container for logging.
3. If the visibility start time is missing, log an error and return false.
4. If the pcal start time is missing, assume it equals the visibility start time (with a warning).
5. Extract the time axes from both containers and verify each has at least two points (needed to compute the accumulation period).
6. Compute the accumulation period (AP) for both datasets:

   delta_t_vis = t_vis[1] - t_vis[0]
   delta_t_pcal = t_pcal[1] - t_pcal[0]

   and check that \|delta_t_vis - delta_t_pcal\| <= eps_AP (default eps_AP = 0.01 s). If the APs differ beyond tolerance, log an error and return false.
7. Parse the VEX start times into ``hops_clock`` time points. Compute the start-time difference:

   delta_t_start = t_pcal_start - t_vis_start - delta_t_pcal / 2

   The subtraction of delta_t_pcal / 2 accounts for the fact that pcal data uses time centroids.
8. If \|delta_t_start\| < eps_start * delta_t_vis (default eps_start = 0.9) **and** the number of time points is identical, no trimming is needed and the operator returns true immediately.
9. Otherwise, compute the visibility time range as absolute time points:

   t_vis_first = t_vis_start + t_vis[0] * 1e9 (nanoseconds)
   t_vis_last = t_vis_start + (t_vis[N-1] + delta_t_vis) * 1e9

10. Build a selection list by iterating over all pcal time points and keeping only those whose absolute time falls within [t_vis_first, t_vis_last]:

    t_pcal(i) = t_pcal_start + t_pcal[i] * 1e9

11. Create an ``MHO_SelectRepack< multitone_pcal_type >`` operator, configure it with the selection list on the ``MTPCAL_TIME_AXIS``, and execute both ``Initialize`` and ``Execute`` to perform the in-place trimming.
12. Log the original and trimmed pcal dimensions and the new start time.

Effect on Data
--------------
The operator modifies the ``multitone_pcal_type`` container in place by removing
accumulation periods whose absolute times fall outside the visibility time
range. After execution, the pcal data's time axis contains only those APs that
overlap with the visibility data's time span. The number of APs is reduced from
the original count to the count of selected APs. The operator does not modify
visibility data or data weights. If no pcal APs fall within the visibility
range, the operator logs a warning but does not fail (the pcal container may
become empty on the time axis).
