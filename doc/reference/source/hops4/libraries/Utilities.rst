..  _Utilities:

Utilities
=========


Date and time handling
----------------------

The date and time labeling of visibility and other data is an important
component of the hops object specification. Within a scan, the time axis
of any data container will be labeled with a ``double`` precision float
specifying the number of seconds since the scan epoch. Nominally the
scan epoch is the start of the scan as described in the VEX schedule and
thus the first element on the time axis will typically have
:math:`t=0.0`. However, this is not a requirement, and any positive or
negative time offset representable in ``double`` precision is allowed.

The time point representing the date/time of the scan epoch will have an
internal representation within HOPS as a 64-bit integer (``int64_t``
count of the number of UTC nanoseconds since the J2000 epoch. This
provides nanosecond accurate precision for scan-epoch time stamps until
2292. For in-memory manipulation of time and dates, time points will be
represented by:

::

    std::chrono::time_point<hops_clock>;

where the ``hops_clock`` is a UTC based clock with a start epoch of
(January 1, 2000, 11:58:55.816 UTC). When a time point is stored as a
object tag (either in memory, or on disk) for a data container, the time
point class will be converted to a string representation. There will be
a limited number of supported representations. These are described in
the following table (subject to change).

For ease of use, ``hops_clock`` will also support the conversion of time
points into TAI and GPS time. To do this, HOPS will make use of the STL
chrono library and the ``date`` library. As the date library has been
adopted into the C++20 standard, eventually this library will be
eliminated in favor of the compiler implementation. In addition
conversion to and from the legacy HOPS date struct:

::

   typedef struct date
   {
       short   year;
       short   day;
       short   hour;
       short   minute;
       float   second;
   } date_struct;

will also be supported to the degree of precision that the legacy
format allows.


Lets's use chapter to add some C++ code here:
With ``.. code-block:: cpp`` we can add C++ snippets:

.. code-block:: cpp 

    int main()
    {
    std::cout << "hello sphinx!\n";
    return 0;
    }


Source Code documentation
-------------------------

.. doxygenclass:: hops::MHO_DirectoryInterface
    :project: hops
    :members:
    :private-members:


.. doxygenclass:: hops::MHO_FileKey
    :project: hops
    :members:
    :private-members:

.. doxygenclass:: hops::MHO_TimeStampConverter
    :project: hops
    :members:
    :private-members:
