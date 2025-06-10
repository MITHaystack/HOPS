Data Object: `station_data`
===========================

The station_data table container is use to store the information about the a priori station model.
Mainly this is the delay and coordinate polynomial spline models used for the station during correlation.
The data stored in the table are the polynomial coefficients for each coordinate, time period, and polynomial power. 
This structure represents a 3-dimensional dataset with labeled axes:
- **Axis 0** labels the type of station model component.
- **Axis 1** corresponds to spline intervals (time/model epoch).
- **Axis 2** indexes the individual spline coefficients for each power.


General Information
-------------------
- **Class**: :hops:`MHO_TableContainer<double,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<int> > >`
- **Class UUID**: 769cfb37e4301e9272adcb53f217de1b
- **Rank**: 3
- **Shape**: the length of each dimension (e.g. [7, 2, 6] )
- **Strides**: the stride between data in each dimension (e.g. [12, 6, 1] )
- **Total Size**: the total number of data elements in the array (e.g. 84 )
- **Element Type**: ``double``

Tags
----

The meta-data tags (key:value pairs) that are commonly associated with objects 
of this type are as follows:

.. list-table::
   :header-rows: 1

   * - key
     - description
     - example value
   * - name
     - the container name
     - station_data
   * - X
     - station X coordinate
     - -5543831.705
   * - Y
     - station Y coordinate
     - -2054585.983
   * - Z
     - station Z coordinate
     - 2387828.776
   * - difx_station_code
     - the DiFX formatted 2-char station code
     - K2
   * - model_interval
     - the time length of each polynomial model period (s)
     - 120.0
   * - model_start
     - the start time of the spline model (vex format)
     - 2019y105d18h00m00s
   * - mount
     - the station mount type
     - AZEL
   * - nsplines
     - the number of spline intervals
     - 2
   * - station_code
     - The cannonical 2-char station code
     - K2
   * - station_mk4id
     - the 1-char mark4 station ID
     - H
   * - station_name
     - the full station name
     - KOKEE12M

Axes
----

+------------+-------------------------+----------------+--------------------------+---------------------------------------------------------------------------+
| Axis Index | Name                    | Units          | Type                     | Description/Example                                                       |
+============+=========================+================+==========================+===========================================================================+
| 0          | coordinate              | N/A            | ``MHO_Axis<string>``     |   ["delay", "azimuth", "elevation", "parallactic_angle", "u", "v", "w"]   |
+------------+-------------------------+----------------+--------------------------+---------------------------------------------------------------------------+
| 1          | time                    | s              | ``MHO_Axis<double>``     | start time offset of each spline period (0, 120.0,...)                    |
+------------+-------------------------+----------------+--------------------------+---------------------------------------------------------------------------+
| 2          | polynomial_spline_index | N/A            | ``MHO_Axis<int>``        | coefficient power 0, 1, 2,...                                             |
+------------+-------------------------+----------------+--------------------------+---------------------------------------------------------------------------+
