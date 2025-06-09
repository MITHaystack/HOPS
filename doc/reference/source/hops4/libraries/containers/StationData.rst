Station Data Table: `station_data`
==================================

A 3D data container representing station model data using the `MHO_TableContainer<double, MHO_AxisPack<...>>` class.

Container Summary
-----------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Attribute
     - Description
   * - **Class Name**
     - ``MHO_TableContainer<double, MHO_AxisPack<MHO_Axis<string>, MHO_Axis<double>, MHO_Axis<int>>>``
   * - **Class UUID**
     - ``769cfb37e4301e9272adcb53f217de1b``
   * - **Rank**
     - 3
   * - **Dimensions**
     - ``[7, 2, 6]`` (axis_0 × axis_1 × axis_2)
   * - **Strides**
     - ``[12, 6, 1]``
   * - **Total Size**
     - 84

Metadata Tags
-------------

.. list-table::
   :widths: 25 75

   * - ``X``
     - -5543831.705
   * - ``Y``
     - -2054585.983
   * - ``Z``
     - 2387828.776
   * - ``difx_station_code``
     - ``K2``
   * - ``model_interval``
     - 120.0 seconds
   * - ``model_start``
     - ``2019y105d18h00m00s``
   * - ``mount``
     - ``AZEL``
   * - ``name``
     - ``station_data``
   * - ``nsplines``
     - 2
   * - ``station_code``
     - ``K2``
   * - ``station_mk4id``
     - ``H``
   * - ``station_name``
     - ``KOKEE12M``

Axes
----

**Axis 0** – Coordinate Type
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 25 75

   * - **Class**
     - ``MHO_Axis<string>``
   * - **UUID**
     - ``bbd8bbb8d7fdd9ac34e923c09d3fae74``
   * - **Rank**
     - 1
   * - **Dimensions**
     - ``[7]``
   * - **Total Size**
     - 7
   * - **Data**
     - ``["delay", "azimuth", "elevation", "parallactic_angle", "u", "v", "w"]``

**Axis 1** – Time Interval (Seconds)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 25 75

   * - **Class**
     - ``MHO_Axis<double>``
   * - **UUID**
     - ``1169c341834e43322c0edb4948908121``
   * - **Rank**
     - 1
   * - **Dimensions**
     - ``[2]``
   * - **Total Size**
     - 2
   * - **Data**
     - ``[0.0, 120.0]``

**Axis 2** – Polynomial Spline Index
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 25 75

   * - **Class**
     - ``MHO_Axis<int>``
   * - **UUID**
     - ``3cf7068251864a751caf66eac1fb3f9a``
   * - **Rank**
     - 1
   * - **Dimensions**
     - ``[6]``
   * - **Total Size**
     - 6
   * - **Data**
     - ``[0, 1, 2, 3, 4, 5]``

Notes
-----

This structure represents a 3-dimensional dataset with labeled axes:
- **Axis 0** labels the type of station model component.
- **Axis 1** corresponds to spline intervals (time/model epoch).
- **Axis 2** indexes individual spline coefficients or time steps.

