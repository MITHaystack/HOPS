..  _dfio:

dfio - Mark4 Correlator I/O types
=================================

The Mk4 data structures are divided up into three main categories. These are correlator output, station data, and fringe output.
The correlator output structures all have names of the form type_1XX, the station data structures all have names of the form type_3XX
while the fringe data structures are all named in the form type_2XX.


Generic types
-------------

**type_000**

The ``type_000`` struct contains information about the file creation date and the scan name.
See :hops:`type_000` for more information. 

Correlator output types
-----------------------

**type_100**  

The ``type_100`` struct contains meta-data about the correlation, such as the start/stop time, the number of *lags* in the visibility
data records, and the number of visibility data records.

See :hops:`type_100` for more information.

**type_101**  

The ``type_101`` struct contains meta-data the associated visibility record which allows one to map the reference/remote station 
channel names to the index into the record array. There is one ``type_101`` record for each pair of correlated channels.

See :hops:`type_101` for more information.

**type_110**  

The ``type_110`` records are specific to the Mark4 hardware correlator and are deprecated.

See :hops:`type_110` for more nformation.

**type_120**  

The ``type_120`` records contain the correlated visibilities. There is one record for every AP of each correlated channel pair.
For example if there 30 1-second APs, and 32 channels for two polarization (X and Y) then there will be a total of 3840 ``type_120`` records.
Each record contains a meta-data header about the baseline, AP, data weight, and visibility data type, along with an array of the complex visibilities. 
The raw complex visibility data imported from DiFX is stored as a pair of floats in the union element ``lag_data`` and takes the form of:

.. code-block:: cpp 

    struct spectral
    {
        float re;
        float im;
    };

See :hops:`type_120` for more information.

Station data types
------------------

**type_300**  

The ``type_300`` struct contains meta-data pertaining to the station spline models used for the a priori delay and other station coordinates.

See :hops:`type_300` for more information.


**type_301**  

The ``type_301`` struct contains the coefficients of the a priori delay spline used for each channel of the respective station.

See :hops:`type_301` for more information.

**type_302**  

The ``type_302`` struct contains the coefficients of the a priori phase spline used for each channel of the respective station. 
This model is not independent and can be derived from the delay model and the channel frequency.

See :hops:`type_302` for more information.

**type_303**

The ``type_303`` struct contains the spline coefficients of the a priori model for each channel of the respective station
for the following coordinate quantities:

  - azimuth 
  - elevation 
  - parallactic_angle
  - u coordinate 
  - v coordinate 
  - w coordinate

See :hops:`type_303` for more information.

**type_304**

The ``type_304`` struct is for Mark4 correlator CRC error tracking and is **deprecated**.

See :hops:`type_304` for more information.

**type_305**  

The ``type_304`` struct is *unused*.

See :hops:`type_305` for more information.

**type_306**  

The ``type_306`` struct is for Mark4 correlator state count tracking and is **deprecated**.

See :hops:`type_306` for more information.

**type_307**  

The ``type_307`` struct is for Mark4 correlator phase-calibration extraction and is **deprecated**.

See :hops:`type_307` for more information.

**type_308**  

The ``type_308`` struct is for storage of phase calibration data and is **deprecated**.

See :hops:`type_308` for more information.

**type_309**  

The ``type_309`` struct is used for the storage of multi-tone phase calibration data.

See :hops:`type_309` for more information.

Fringe (fourfit) output data types
----------------------------------

**type_200**  

See :hops:`type_200` for more information.

**type_201**  

See :hops:`type_201` for more information.

**type_202**  

See :hops:`type_202` for more information.

**type_203**  

See :hops:`type_203` for more information.

**type_204**  

See :hops:`type_204` for more information.

**type_205**  

See :hops:`type_205` for more information.

**type_206**  

See :hops:`type_206` for more information.

**type_207**  

See :hops:`type_207` for more information.

**type_208**  

See :hops:`type_208` for more information.

**type_210**  

See :hops:`type_210` for more information.

**type_212**  

See :hops:`type_212` for more information.

**type_220**  

See :hops:`type_220` for more information.

**type_221**  

See :hops:`type_221` for more information.

**type_222**  

See :hops:`type_222` for more information.

**type_230**  

See :hops:`type_230` for more information.






