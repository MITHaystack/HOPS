Data Object: visibilities
=========================

General Metadata
----------------
- **Class**: :hops:`hops::MHO_TableContainer<float,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>`
- **UUID**: a5c26065821b6dc92b06f780f8641d0e
- **Rank**: 4
- **Shape**: [4, 32, 30, 128]
- **Strides**: [122880, 3840, 128, 1]
- **Total Size**: 491520

**Tags**

.. list-table::
   :header-rows: 1

   * - key
     - value
   * - baseline
     - Gs-Wf
   * - baseline_shortname
     - GE
   * - reference_station
     - GGAO12M
   * - reference_station_mk4id
     - G
   * - remote_station
     - WESTFORD
   * - remote_station_mk4id
     - E
   * - correlation_date
     - 2023y061d02h05m31s
   * - start_time
     - 2019y105d18h00m00s
   * - origin
     - difx
   * - difx_baseline_index
     - 260
   * - root_code
     - 3SMFW3


Axes Overview
-------------

+------------+----------------------+----------------+------------+-----------------------------------+
| Axis Index | Name                 | Units          | Type       | Description                       |
+============+======================+================+============+===================================+
| 0          | polarization_product | N/A            | string     | XX, XY, YY, RR, LL, XR, YL        |
+------------+----------------------+----------------+------------+-----------------------------------+
| 1          | channel              | MHz            | double     | lower channel edge (e.g. 3032.4)  |
+------------+----------------------+----------------+------------+-----------------------------------+
| 2          | time                 | s              | double     | Start of each AP (0, 1.0,...)     |
+------------+----------------------+----------------+------------+-----------------------------------+
| 3          | frequency            | MHz            | double     | The intra-channel frequency       |
+------------+----------------------+----------------+------------+-----------------------------------+

Channel Axis (Index 1) Detailed Tags
------------------------------------

The index tag entries along the channel axis includes:

- **channel_id**: the reference and remote channel id pairs (e.g. `X04LX:X04LY`)
- **bandwidth**: float (MHz) e.g. 32.0 MHz
- **difx_frequency_ndex**: integer (e.g. 0-31)
- **frequency_band**: X
- **net_sideband**: L (lower) or U (upper)
- **sky_frequency**: float (MHz) e.g. 3032.4

Example entries:

+--------+----------------+---------+--------------+------------------+
| Index  | Sky Freq (MHz) | Channel | DIFX Index   | Ref:Rem MK4 ID   |
+========+================+=========+==============+==================+
| 0      | 3032.4         | 0       | 7            | X00LX:X00LX      |
+--------+----------------+---------+--------------+------------------+
| 1      | 3064.4         | 1       | 6            | X01LX:X01LX      |
+--------+----------------+---------+--------------+------------------+
| 2      | 3096.4         | 2       | 5            | X02LX:X02LX      |
+--------+----------------+---------+--------------+------------------+
| ...    | ...            | ...     | ...          | ...              |
+--------+----------------+---------+--------------+------------------+
| 31     | 10680.4        | 31      | 24           | X31LX:X31LX      |
+--------+----------------+---------+--------------+------------------+
