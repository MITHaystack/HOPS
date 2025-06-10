Data Object: `visibilities`
===========================

The visibility table container is use to store the raw correlator visibilities. 
On disk the data elements of this object are ``std::complex<float>``, but these 
are promoted to ``std::complex<double>`` for in-memory processing.


General Information
-------------------
- **Class**: :hops:`hops::MHO_TableContainer<std::complex<float>,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double> > >`
- **Class UUID**: a5c26065821b6dc92b06f780f8641d0e
- **Rank**: 4
- **Shape**: the length of each dimension (e.g. [4, 32, 30, 128] )
- **Strides**: the stride between data in each dimension (e.g. [122880, 3840, 128, 1] )
- **Total Size**: the total number of data elements in the array (e.g. 491520 )
- **Element Type**: ``std::complex<float>``

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
     - visibilities
   * - baseline
     - the baseline name (using 2-char station codes)
     - Gs-Wf
   * - baseline_shortname
     - the condensed baseline name (using 1-char mk4 station codes)
     - GE
   * - reference_station
     - the reference station name
     - GGAO12M
   * - reference_station_mk4id
     - the reference station 1-char mk4 ID code
     - G
   * - remote_station
     - the remote station name
     - WESTFORD
   * - remote_station_mk4id
     - the remote station 1-char mk4 ID code
     - E
   * - correlation_date
     - time of correlation (in vex format)
     - 2023y061d02h05m31s
   * - start
     - the scan start time (in vex format)
     - 2019y105d18h00m00s
   * - stop
     - the scan stop time (in vex format)
     - 2019y105d18h00m30s
   * - origin
     - the source of the data
     - difx or mark4
   * - difx_baseline_index
     - if the data was imported from DiFX, the integer baseline code
     - 260
   * - root_code
     - the mk4 style 6-char root code
     - 3SMFW3


Axes
----

+------------+----------------------+----------------+--------------------------+-----------------------------------+
| Axis Index | Name                 | Units          | Type                     | Description/Example               |
+============+======================+================+==========================+===================================+
| 0          | polarization_product | N/A            | ``MHO_Axis<string>``     | XX, XY, YY, RR, LL, XR, YL        |
+------------+----------------------+----------------+--------------------------+-----------------------------------+
| 1          | channel              | MHz            | ``MHO_Axis<double>``     | lower channel edge (e.g. 3032.4)  |
+------------+----------------------+----------------+--------------------------+-----------------------------------+
| 2          | time                 | s              | ``MHO_Axis<double>``     | start of each AP (0, 1.0,...)     |
+------------+----------------------+----------------+--------------------------+-----------------------------------+
| 3          | frequency            | MHz            | ``MHO_Axis<double>``     | the intra-channel frequency       |
+------------+----------------------+----------------+--------------------------+-----------------------------------+

Channel Axis (Index 1) - Detail
-------------------------------

Each axis is capable of having additional data appended in the form of index tags,
or interval tags. By default both ``difx2hops`` and ``mark42hops`` will append the channel 
axis with additional index tags which contain information needed to process the visibility data.
The index tag entries along the channel axis include *(but are not limited to)* the following:

- **channel_id**: the reference/remote channel id pairs (e.g. `X04LX:X04LY`)
- **bandwidth**: float (MHz) e.g. 32.0 MHz
- **difx_frequency_index**: integer (e.g. 0-31)
- **frequency_band**: X
- **net_sideband**: L (lower) or U (upper)
- **sky_frequency**: float (MHz) e.g. 3032.4

An example of such index tag entries for the channel axis could look like:

+--------+----------------+--------------+--------------+-------------+------------------+
| index  | sky_frequency  | net_sideband |  bandwidth   | DIFX Index  | channel_id       |
+========+================+==============+==============+=============+==================+
| 0      | 3032.4         | L            |     32       |7            | X00LX:X00LX      |
+--------+----------------+--------------+--------------+-------------+------------------+
| 1      | 3064.4         | L            |     32       |6            | X01LX:X01LX      |
+--------+----------------+--------------+--------------+-------------+------------------+
| 2      | 3096.4         | L            |     32       |5            | X02LX:X02LX      |
+--------+----------------+--------------+--------------+-------------+------------------+
| ...    | ...            | ...          |     ...      |...          | ...              |
+--------+----------------+--------------+--------------+-------------+------------------+
| 31     | 10680.4        | L            |     32       |24           | X31LX:X31LX      |
+--------+----------------+--------------+--------------+-------------+------------------+
