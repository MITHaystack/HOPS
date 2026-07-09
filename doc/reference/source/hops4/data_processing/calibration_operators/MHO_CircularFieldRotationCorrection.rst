MHO\_CircularFieldRotationCorrection
======================================

Purpose
-------

This operator applies a circular-polarization field-rotation correction to
visibility data. It compensates for the differential parallactic-angle rotation
between the reference and remote stations, which arises because each antenna's
feed system imposes a rotation on the Stokes frame as the source tracks across
the sky. The correction is essential when using circular-polarization feeds
(such as those on Nasmyth-mounted antennas) and ensures that the visibility
products remain in a common polarization reference frame.

Control File Trigger
--------------------

This operator is triggered by the ``circ_field_rotation_corr`` keyword in the
control file. It belongs to the calibration category with priority 3.98. The
operator takes no JSON parameters; it is configured entirely from station
metadata (mount type and coordinate data) and the polarization product set
already present in the parameter store.

The operator requires the following parameters to be available in the parameter store:

- ``/config/polprod_set`` -- list of polarization product labels (e.g., ``LL``, ``RR``, ``LR``, ``RL``)
- ``/vex/scan/fourfit_reftime`` -- VEX-formatted reference time string for evaluating station coordinates
- ``/control/station/mount_type`` -- antenna mount type (may be specified generically or per-station as ``/control/station/<site_id>/mount_type``)

The supported mount types are: ``no_mount`` (or empty string), ``cassegrain``, ``nasmythleft``, and ``nasmythright``. If no mount-type information is available for either station, the operator is not built.

Input Data
----------

The operator works in-place on a ``visibility_type`` container. The container
is indexed over polarization products, channels, accumulation periods, and
intra-channel frequency bins. Additionally, the operator requires
``station_coord_type`` objects for both the reference and remote stations
(retrieved from the container store as ``ref_sta`` and ``rem_sta``) to evaluate
parallactic angles and elevations.

Algorithm
---------

The operator uses ``MHO_StationModel`` to compute the parallactic angle (:math:`\gamma`) and elevation (:math:`\text{el}`) for both the reference and remote stations at the fourfit reference time. These angular values (in degrees) are converted to radians before use.

Each antenna mount type has an associated elevation multiplier, :math:`e_m`, which determines how the antenna's mechanical feed rotation contributes to the net parallactic-angle offset:

.. math::

   e_m = \begin{cases}
    0      & \text{for no\_mount or cassegrain} \\
   -1      & \text{for nasmythleft} \\
   +1      & \text{for nasmythright}
   \end{cases}

For each polarization product :math:`pp \in \{\text{LL}, \text{RR}, \text{LR}, \text{RL}\}`, the operator computes a net rotation angle, :math:`\Theta_{pp}`, by combining the parallactic angle and elevation from both stations, each scaled by the respective mount-type multiplier:

.. math::

   \alpha_i = \gamma_i + e_{m,i} \cdot \text{el}_i \qquad (i = 0 \text{ for reference}, \; i = 1 \text{ for remote})

.. math::

   \Theta_{\text{LL}} = +\alpha_0 - \alpha_1

.. math::

   \Theta_{\text{RR}} = -\alpha_0 + \alpha_1

.. math::

   \Theta_{\text{LR}} = +\alpha_0 + \alpha_1

.. math::

   \Theta_{\text{RL}} = -\alpha_0 - \alpha_1

The complex pre-factor for each polarization product is then:

.. math::

   F_{pp} = \exp\!\bigl(-j \cdot \Theta_{pp}\bigr)

where :math:`j` is the imaginary unit. The visibility sub-view for each polarization product is multiplied in-place by its corresponding pre-factor.

Effect on Data
--------------

This operator modifies the input visibility container in-place. For each
polarization product present in the configured product set, all visibility
samples (across channels, accumulation periods, and frequency bins) are
multiplied by a complex pre-factor :math:`F_{pp} = \exp(-j \cdot \Theta_{pp})`.
This pre-factor rotates the circular-polarization products into a common
reference frame, compensating for the differential parallactic-angle rotation
between the two stations. The container's axis labels and metadata are unchanged.

Important! This operation is approximate, and treats the rotation as a single
static value over the course of a scan, for long scans this may be incorrect.
