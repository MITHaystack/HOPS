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
See :hops:`type_000` for more type_000 information. 

Correlator output types
-----------------------

**type_100**  

The ``type_100`` struct contains meta-data about the correlation, such as the start/stop time, the number of *lags* in the visibility
data records, and the number of visibility data records.

See :hops:`type_100` for more type_100 information.

**type_101**  

The ``type_101`` struct contains meta-data the associated visibility record which allows one to map the reference/remote station 
channel names to the index into the record array. There is one ``type_101`` record for each pair of correlated channels.

See :hops:`type_101` for more type_101 information.

**type_110**  

The ``type_110`` records are specific to the Mark4 hardware correlator and are deprecated.

See :hops:`type_110` for more type_110 information.

**type_120**  

The ``type_120`` records contain the correlated visibilities. There is one record for every AP of each correlated channel pair.
For example if there 30 1-second APs, and 32 channels for two polarization (X and Y) then there will be a total of 3840 ``type_120`` records.
Each record contains a meta-data header about the baseline, AP, data weight, and visibility data type, along with an array of the complex visibilities. 
The raw visibility data imported from DiFX is stored in the ``union lag_data`` and takes the form of

::

struct spectral
    {
    float re;
    float im;
    };


See :hops:`type_120` for more type_120 information.

Station data types
------------------

**type_300**  

See :hops:`type_300` for more type_300 information.

**type_301**  

See :hops:`type_301` for more type_301 information.

**type_302**  

See :hops:`type_302` for more type_302 information.

**type_303**

See :hops:`type_303` for more type_303 information.

**type_304**

See :hops:`type_304` for more type_304 information.

**type_305**  

See :hops:`type_305` for more type_305 information.

**type_306**  

See :hops:`type_306` for more type_306 information.

**type_307**  

See :hops:`type_307` for more type_307 information.

**type_308**  

See :hops:`type_308` for more type_308 information.

**type_309**  

See :hops:`type_309` for more type_309 information.

Fringe (fourfit) output data types
----------------------------------

**type_200**  

See :hops:`type_200` for more type_200 information.

**type_201**  

See :hops:`type_201` for more type_201 information.

**type_202**  

See :hops:`type_202` for more type_202 information.

**type_203**  

See :hops:`type_203` for more type_203 information.

**type_204**  

See :hops:`type_204` for more type_204 information.

**type_205**  

See :hops:`type_205` for more type_205 information.

**type_206**  

See :hops:`type_206` for more type_206 information.

**type_207**  

See :hops:`type_207` for more type_207 information.

**type_208**  

See :hops:`type_208` for more type_208 information.

**type_210**  

See :hops:`type_210` for more type_210 information.

**type_212**  

See :hops:`type_212` for more type_212 information.

**type_220**  

See :hops:`type_220` for more type_220 information.

**type_221**  

See :hops:`type_221` for more type_221 information.

**type_222**  

See :hops:`type_222` for more type_222 information.

**type_230**  

See :hops:`type_230` for more type_230 information.






