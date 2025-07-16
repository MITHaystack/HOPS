Non-Operator Utility Classes
============================

MHO_DelayModel
--------------

======================= ======================================================
Class                   :hops:`MHO_DelayModel`
Required Input          :hops:`station_coord_type`
======================= ======================================================

Evaluates the station a priori delay model polynomials for both reference and 
remote stations. Computes delay, delay-rate, and delay-acceleration, evaluated 
at the fourfit reference time.

MHO_StationModel
----------------

======================= ======================================================
Class                   :hops:`MHO_StationModel`
Required Input          :hops:`station_coord_type`
======================= ======================================================

Evaluates the station a priori coordinate and/or delay spline polynomials to 
compute station model parameters including delay, azimuth, elevation, and 
(approximate) parallactic angle.
