Data Object: `pcal`
===================

The pcal table container is use to store the multi-tone phase calibration tone phasor data.
This table has rank 3, and the data is labeled by station polarization, time, and tone frequency.
It is used by the :hops:`hops::MHO_MultitonePhaseCorrection` calibration operator.

General Information
-------------------
- **Class**: :hops:`multitone_pcal_type`
- **Class UUID**: 61831c0460859e21f30799d59d491fdf
- **Rank**: 3
- **Shape**: the length of each dimension (e.g. [2, 30, 204] )
- **Strides**: the stride between data in each dimension (e.g. [6120, 204, 1] )
- **Total Size**: the total number of data elements in the array (e.g. 12240 )
- **Element Type**: ``std::complex<double>``

**Aliases**

.. list-table::
   :header-rows: 1

   * - alias
     - full class name
   * - ``multitone_pcal_type``
     - ``hops::MHO_TableContainer<std::complex<double>,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>>>``


Tags
----

The meta-data tags (key:value pairs) that are commonly associated with objects 
of this type are as follows:

.. list-table::
   :header-rows: 1

   * - name
     - the container name
     - pcal
   * - key
     - description
     - example value
   * - difx_station_code
     - The DiFX formatted 2-char station code
     - GS
   * - pcal_accumulation_period
     - the time averaging period (s) for tone phasors
     - 1.0
   * - start
     - the start time of the pcal data (in vex format)
     - 2019y105d18h00m00.501120128s
   * - start_time_mjd
     - the start time of the pcal data (modfied Julian day format (DiFX default))
     - 58588.7500058
   * - station_code
     - The cannonical 2-char station code
     - Gs
   * - station_mk4id
     - the 1-char mark4 station ID
     - H
   * - station_name
     - the full station name
     - KOKEE12M



Axes
----

+------------+----------------------+----------------+--------------------------+-----------------------------------+
| Axis Index | Name                 | Units          | Type                     | Description/Example               |
+============+======================+================+==========================+===================================+
| 0          | polarization         | N/A            | ``MHO_Axis<string>``     | X, Y, R, L,                       |
+------------+----------------------+----------------+--------------------------+-----------------------------------+
| 2          | time                 | s              | ``MHO_Axis<double>``     | start of each AP (0, 1.0,...)     |
+------------+----------------------+----------------+--------------------------+-----------------------------------+
| 3          | frequency            | MHz            | ``MHO_Axis<double>``     | the tone frequency                |
+------------+----------------------+----------------+--------------------------+-----------------------------------+
