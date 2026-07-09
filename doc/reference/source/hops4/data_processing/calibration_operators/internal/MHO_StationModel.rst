MHO\_StationModel
==================

Purpose
-------

This class evaluates a station's a priori coordinate and delay model from
spline coefficients stored in a ``station_coord_type`` container. It computes
the geometric delay, source azimuth, source elevation, parallactic angle,
and the (u,v,w) baseline coordinates at a specified evaluation time. It is
an internal utility with no control file keyword.

Control File Trigger
--------------------

This operator is internal and has no control file keyword. It is used by
higher-level operators (e.g., fringe-fitting and delay-modeling pipelines)
to obtain the geometric delay model for each station in a baseline.

Input Data
----------

The class takes a pointer to a ``station_coord_type`` container (a two-axis structure) via ``SetStationData()``. The container's axes are:

- **Row axis (coordinate type)** -- 7 rows indexed as: DELAY (0), AZIMUTH (1), ELEVATION (2), PARANGLE (3), U (4), V (5), W (6).
- **INTERVAL\_AXIS** -- spline intervals (each interval holds a set of polynomial coefficients).

Each cell contains a vector of spline coefficients (polynomial terms ordered from p=0 upward).
The container also carries metadata
tags: ``station_code`` (string), ``model_start`` (VEX-format time string),
and ``model_interval`` (double, seconds).

Algorithm
---------

The ``ComputeModel()`` method performs the following steps:

**Step 1 -- Time Setup.**

The model start time is retrieved from the ``model_start`` tag and parsed from VEX format using ``hops_clock::from_vex_format()``. The evaluation time is either user-supplied via ``SetEvaluationTimeVexString()`` or defaults to the model start time if not set. The time difference :math:`\Delta t = t_{\rm eval} - t_{\rm start}` is computed in seconds.

**Step 2 -- Spline Interval Selection.**

The model interval duration is retrieved from the ``model_interval`` tag. The spline interval index is computed as:

.. math::

   n_{\rm interval} = \left\lfloor \frac{\Delta t}{\Delta t_{\rm interval}} \right\rfloor

The ``CheckSplineInterval()`` method clamps the interval to the valid range [0, N\_intervals-1], issuing a warning if extrapolation is required (either :math:`\Delta t < 0` or :math:`n_{\rm interval} \geq N_{\rm intervals}`).

**Step 3 -- Time Offset Within Interval.**

The time offset within the selected interval is:

.. math::

   \delta t = \Delta t - n_{\rm interval} \cdot \Delta t_{\rm interval}

**Step 4 -- Polynomial Evaluation.**

For each of the 7 coordinate types (delay, azimuth, elevation, parallactic angle, u, v, w), the operator extracts the spline coefficient vector for the selected interval and evaluates the polynomial:

.. math::

   \mathrm{coord} = \sum_{p=0}^{N_{\rm coeff}-1} c_p \cdot (\delta t)^p

where :math:`c_p` is the p-th coefficient in the spline's coefficient vector. This is a standard polynomial evaluation (implemented as a direct sum of terms).

.. note::
   The parallactic angle evaluation does not produce a meaningful result, since
   CALC does not provide a genuine spline for this coordinate. The value returned
   by ``GetParallacticAngle()`` should not be relied upon; a proper calculation
   from azimuth, elevation, and station coordinates is still pending.

Effect on Data
--------------

This class does not modify its input container. After ``ComputeModel()`` is
called, the computed values (delay, azimuth, elevation, parallactic angle, u, v, w)
are stored as private member variables and are retrievable via
the ``GetDelay()``, ``GetAzimuth()``, ``GetElevation()``, ``GetParallacticAngle()``, ``GetUCoordinate()``, ``GetVCoordinate()``, and ``GetWCoordinate()`` methods.
