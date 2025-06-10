Data Object: `pcal`
===================

A complex-valued data container representing phase calibration data using the
``MHO_TableContainer<std::complex<double>, MHO_AxisPack<...>>`` class.

Container Summary
-----------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Attribute
     - Description
   * - **Class Name**
     - ``MHO_TableContainer<std::complex<double>, MHO_AxisPack<MHO_Axis<string>, MHO_Axis<double>, MHO_Axis<double>>>``
   * - **Class UUID**
     - ``61831c0460859e21f30799d59d491fdf``
   * - **Rank**
     - 3
   * - **Dimensions**
     - ``[2, 30, 204]`` (axis_0 × axis_1 × axis_2)
   * - **Strides**
     - ``[6120, 204, 1]``
   * - **Total Size**
     - 12,240 (complex elements)

Metadata Tags
-------------

.. list-table::
   :widths: 30 70

   * - ``difx_station_code``
     - ``K2``
   * - ``name``
     - ``pcal``
   * - ``pcal_accumulation_period``
     - 1.0 second
   * - ``start``
     - ``2019y105d18h00m00.501120128s``
   * - ``start_time_mjd``
     - 58588.7500058
   * - ``station_code``
     - ``K2``
   * - ``station_mk4id``
     - ``H``
   * - ``station_name``
     - ``KOKEE12M``

Axes
----

The axes are as follows:

**Axis 0** – Polarization Products
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Represents the polarization of the data (e.g., "R/L", or "X/Y").

- **Size**: 2  
- **Type**: `MHO_Axis<string>`  
- **Contents**: Not included in JSON (inferred)

**Axis 1** – Time Steps
^^^^^^^^^^^^^^^^^^^^^^^

Likely corresponds to time samples during the PCAL accumulation period.

- **Size**: 30  
- **Type**: `MHO_Axis<double>`  
- **Unit**: Seconds since `start`  
- **Resolution**: ~1.0 s / 30 = ~33 ms if evenly spaced

**Axis 2** – Tone Frequencies
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Corresponds to PCAL tone frequencies.

- **Size**: 204  
- **Type**: `MHO_Axis<double>`  
- **Unit**: MHz

Notes
-----

This structure contains a dense, 3D array of complex values representing PCAL amplitude and phase vs. time and tone.

Intended for downstream processing in visibility calibration pipelines.

If axis metadata is available separately, link or embed it alongside this documentation.
