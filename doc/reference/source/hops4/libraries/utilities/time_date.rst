Time and Date Utilities
~~~~~~~~~~~~~~~~~~~~~~~~

The utilities library provides time and date handling capabilities via the 
*date* library (developed by H. Hinnant, see <hops_src>/extern/date). This date 
library has been adopted into the CXX20 STL standard, so it is expected that 
eventually the captured copy of this library will be eliminated in favor of the 
compiler implementation. The internal time point representation of the :hops:`hops_clock` class is
a 64-bit integer (``int64_t``) count of the number of UTC nanoseconds since 
the J2000 epoch (January 1, 2000, 11:58:55.816 UTC). This provides nanosecond 
accurate precision for scan-epoch time stamps until 2292. For in-memory 
manipulation of time and dates, time points are represented by:

::

    std::chrono::time_point<hops_clock>;

When a time point is stored as a object tag (either in memory, or on disk) for 
a data container, the time point class will be converted to a string 
representation (see below for supported types), generally speaking this will be
a VEX style time stamp.

:hops:`hops_clock`
------------------

=============================================== ====================================================================
Class                                           :hops:`hops_clock`
Category                                        Time and Date Utilities
Template Parameters                             Duration-based template parameters for time_point operations
Primary Functionality                           Custom clock based on J2000 epoch (UTC nanoseconds since J2000)
Key Features                                    | Conversion between UTC, TAI, GPS, system, and local time
                                                | Support for ISO8601, VEX, VDIF, MJD, and legacy HOPS formats
                                                | Leap second handling and timezone support
=============================================== ====================================================================

The :hops:`hops_clock` class provides a specialized clock implementation that 
measures time in UTC nanoseconds since the J2000 epoch. It offers conversion 
capabilities between different time systems and formats. It is the basis for 
most time-related operations in HOPS4. The available time_point conversion 
routines (betwen clock types) are implemented by the functions below:

.. code:: cpp

    to_utc(const std::chrono::time_point< hops_clock, Duration >&)
    from_utc(const std::chrono::time_point< date::utc_clock, Duration >&)
    to_tai(const std::chrono::time_point< hops_clock, Duration >&)
    from_tai(const std::chrono::time_point< date::tai_clock, Duration >&)
    to_gps(const std::chrono::time_point< hops_clock, Duration >&)
    from_gps(const std::chrono::time_point< date::gps_clock, Duration >&)
    to_sys(const std::chrono::time_point< hops_clock, Duration >&)
    from_sys(const std::chrono::time_point< std::chrono::system_clock, Duration >&)
    to_local(const std::chrono::time_point< hops_clock, Duration >&)
    from_local(const std::chrono::time_point< date::local_t, Duration >&)

Additional routines are available that handle the conversion of 
hops_clock time_point objects to/from time stamps with the following formats:

#. ISO8601
#. VEX
#. MJD
#. (year,floating-point-day)
#. legacy HOPS3 date struct

The conversion routines are implemented by the associated functions listed below:

.. code:: cpp

    static time_point from_iso8601_format(const std::string& timestamp);
    static std::string to_iso8601_format(const time_point& tp);
    static time_point from_hops_format(const std::string& timestamp);
    static std::string to_hops_format(const time_point& tp);
    static time_point from_legacy_hops_date(legacy_hops_date& ldate);
    static legacy_hops_date to_legacy_hops_date(const time_point& tp);
    static time_point from_vdif_format(int& vdif_epoch, int& vdif_seconds);
    static void to_vdif_format(const time_point& tp, int& vdif_epoch, int& vdif_second);
    static time_point from_mjd(const time_point& mjd_epoch, const double& epoch_offset, const double& mjd);
    static double to_mjd(const time_point& mjd_epoch, const double& epoch_offset, const time_point& tp);
    static time_point from_vex_format(const std::string& timestamp);
    static std::string to_vex_format(const time_point& tp, bool truncate_to_nearest_second = false);
    static time_point from_year_fpday(int year, double floating_point_days);


:hops:`MHO_TimeStampConverter`
------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_TimeStampConverter`
Category                                        Time and Date Utilities
Primary Functionality                           Converts between Unix epoch and human-readable timestamps
Key Features                                    | Unix epoch to ISO8601 format conversion
                                                | Nanosecond precision handling
                                                | Bidirectional conversion support
=============================================== ====================================================================

The :hops:`MHO_TimeStampConverter` class handles conversion between 
Unix epoch seconds (with fractional parts) and human-readable ISO8601 formatted 
date strings. This utility is only used by the MHO_DirectoryInterface for 
reading/converting file modification times. 

:hops:`MHO_LegacyDateConverter`
-------------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_LegacyDateConverter`
Category                                        Time and Date Utilities
Template Parameters                             None
Configuration Parameters                        None
Primary Functionality                           Converts between legacy HOPS3 date formats and modern formats
Key Features                                    | Legacy HOPS3 date struct conversion
                                                | ISO8601 and VEX format support
                                                | Current time and epoch retrieval
=============================================== ====================================================================

The :hops:`MHO_LegacyDateConverter` class provides conversion utilities for 
the legacy HOPS3 ``date`` struct:

::

   typedef struct date
   {
       short   year;
       short   day;
       short   hour;
       short   minute;
       float   second;
   } date_struct;

Conversion is done indirectly through the :hops:`legacy_hops_date` struct in 
order to prevent a name collision with the external *date* library.
