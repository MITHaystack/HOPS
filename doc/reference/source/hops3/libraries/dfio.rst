..  _dfio:

dfio - Mark4 Correlator I/O types
=================================



Mk4 File Format Definitions -- CJL May 13 1999
--------------------------------------------------------------

This document defines the detailed file and record formats to be used for the
Mk4 correlator system.  It also provides background information on the Mk4 data
files and software system.

FILESETS AND FILE TYPES
-----------------------

1. MkIII system
---------------

The MkIII system uses three file types, numbered 0, 1 and 2, and referred to
respectively as root, corel and fringe files, all of which are binary format.  A
fileset consists of a single root file, and multiple type-1 and type-2 files all
of which "belong" to a type-0 root file.  The root files contain global
information about the scan, while the type-1 (corel) and type-2 (fringe) files
contain respectively the raw correlation counts plus minimal identification
information, and the results of a fringe-fitting operation.  There is at least
one type-1 per baseline correlated, and one type-2 file per baseline
fringe-fitted.  In general, there may be many type-1 files per baseline, due to
multiple correlations using the same root, and there may be many type-2 files
per type-1 file, due to multiple passes of the fringe-fitting software, and
multiple subsets of data (such as frequency bands) within a single type-1 file.

A naming convention was devised to handle MkIII files in a Unix setting.
Filesets are bound together by a common "root code" filename extension,
indicating to which root the type-1 and type-2 files belong.  This root code is
a time-tag expressed as a 6-digit base-26 number, encoded in lower-case
alphabetic characters, and corresponds to the root creation time in units of 4
seconds.  While somewhat obscure, this 6-character string is concise, unique
under normal circumstances, and has an acceptably long rollover period of about
40 years.

The root file name consists of the source name and the root code, separated by a
single period (period characters in a source name are mapped to underscores).
Corel files consist of the 2-character baseline ID, a fileset sequence number,
and the rootcode, all three fields separated by periods.  Fringe file names
consist of the baseline ID, a single-character frequency group ID, a fileset
sequence number, and the rootcode, all four fields being separated by periods.
The fileset sequence numbers are necessary in order to uniquely identify
multiple instances of type-1 and type-2 files for the same baseline.  Those
familiar with MkIII will recognize the fileset sequence number as the HP-1000
file extent number, carried over into the Unix world.

Below is an example of a MkIII fileset.  Note the multiple type-1 files for the
AB baseline, indicating multiple correlations under the same root.  There are
also multiple runs of fourfit for the AB baseline at S-band.

+----------------+----------------+----------------+
| root file      | type-1 files   | type-2 files   |
+================+================+================+
| 3C345.ngsbdc   | AB.1.ngsbdc    | AB.S.5.ngsbdc  |
|                |                |                |
|                | AC.2.ngsbdc    | AB.X.6.ngsbdc  |
|                |                |                |
|                | BC.3.ngsbdc    | AB.S.7.ngsbdc  |
|                |                |                |
|                | AB.4.ngsbdc    | AC.S.8.ngsbdc  |
|                |                |                |
|                |                | BC.S.9.ngsbdc  |
|                |                |                |
|                |                | BC.X.10.ngsbdc |
+----------------+----------------+----------------+



2. Mk4 modifications and extensions
-----------------------------------

The Mk4 system is modeled after MkIII to a degree.  The concept of root, corel
and fringe files persists, but two additional file types have been added.  These
are the type-3 (station data) files, and the type-4 (log) file.  The type-3
files, one per station, contain model spline coefficients, phasecal data, state
count information, and tape error statistics (in MkIII, some of this information
was embedded in the type-1 file records).  The type 4 file, one per root,
contains generalized ascii log information both from the field system and from the
correlator, specific to the fileset.  In addition to the new file types, the
provision for multiple correlations under the same root file has been removed.

New filenaming conventions have been devised to deal with Mk4 data files.  The
intent is to modify conventions as little as possible, to maximize the utility
of existing postprocessing code, while eliminating redundant or useless
information in the filenames.  The naming conventions for root files and type-2
(fringe) files are unchanged.  Type-1 files no longer need a fileset sequence
number, so it has been removed leaving the baseline ID, two periods, and the
root code.  The double period distinguishes type-1 files from root files with
2-character source names.  Type-3 files have the single-character station ID, a
double period, and the root code.  Type 4 files have the string "log", a period,
and the root code.  An example MK4 fileset is shown below.  Three stations were
correlated in a geodetic S/X experiment, A, B and C.  Baselines AB and BC were
fringe-fitted at both S and X band, and baseline AC only at S-band.  Baseline
AB was refringed at S-band, resulting in file AB.S.3.ngsbdc with a fileset
sequence number of 3.

+----------------+----------------+----------------+----------------+----------------+
| root file      | type-1 files   | type-2 files   | type-3 files   | type-4 file    |
+================+================+================+================+================+
| 3C345.ngsbdc   | AB..ngsbdc     | AB.S.1.ngsbdc  | A..ngsbdc      | log.ngsbdc     |
|                |                |                |                |                |
|                | AC..ngsbdc     | AB.X.2.ngsbdc  | B..ngsbdc      |                |
|                |                |                |                |                |
|                | BC..ngsbdc     | AB.S.3.ngsbdc  | C..ngsbdc      |                |
|                |                |                |                |                |
|                |                | AC.S.4.ngsbdc  |                |                |
|                |                |                |                |                |
|                |                | BC.S.5.ngsbdc  |                |                |
|                |                |                |                |                |
|                |                | BC.X.6.ngsbdc  |                |                |
+----------------+----------------+----------------+----------------+----------------+


3. VEX root files
-----------------

In a major departure from MkIII, root files in the Mk4 system are ascii files in
the VEX format.  VEX is known to the global VLBI community as the language used
to describe how a VLBI experiment will be observed, and which is generated by
scheduling programs.  This public definition of VEX does not include a variety
of parameters needed to run a correlator.  The Mk4 correlator software system
therefore includes definitions for several new flavors of VEX, designed to
configure and run the various subsystems of the Mk4 correlator.  These flavors,
described in detail elsewhere, have been labelled OVEX, IVEX, EVEX, SVEX, CVEX
and LVEX.  The public flavor used for observing is OVEX.  The Mk4 root file is
defined to be the concatenation of all relevant portions of these six flavors of
VEX, which then constitutes a complete description of how the information in the
data files originated, from observation right through the correlator system,
with details specified down to the insides of each correlator chip in the
system.  Much of this information is of diagnostic interest only, but the format
is relatively compact, and a typical root file consumes less than 50 kilobytes.
Root files are read using a VEX parsing and utility library.  Any software
seeking to use Mk4 data will likely need a copy of this VEX library, which is
written in standard C.

BINARY FILE TYPES
-----------------

Only type-1, type-2 and type-3 files are binary.  These files are comprised of a
series of binary records, which with few restrictions may occur in arbitrary
order.  Because of architectural differences between the MkIII and Mk4
correlators, these binary record formats have been developed from scratch, and
bear no relation to MkIII record formats.  Any software which reads Mk4
correlator data files will need a new IO interface at the minimum.  An IO
library has been written which transparently deals with multiple format revision
levels (see below), and which should ease the task of reading and writing Mk4
data files.  The library is written in standard C.

Before defining record formats in detail, we first describe features common to
all binary records and files in the Mk4 system.

GENERAL RECORD CONSIDERATIONS
-----------------------------

1. Record identification
------------------------

In order to facilitate the reading and decoding of Mk4 binary files, all data
records are "typed".  This is done, with one notable exception, in the Mk3
system.  For Mk4, we have devised a typing scheme that allows format evolution,
and which is distinct from the Mk3 system to avoid possible confusion.  Each
record has, in the first 8 bytes, ascii characters which uniquely identify the
record type, and which tell the IO software exactly how to handle it.  The use
of ascii information rather than binary integers allows pure ascii records to be
typed in a uniform manner, yet carries no penalty.  This 8-byte header contains
2 fields.  The first 3 bytes contain the record type number, which in Mk4 cannot
exceed 999.  The next 2 bytes contain a format version number, up to 99.  These
two numbers are formatted with leading zeros.  The remaining 3 bytes are
reserved for future use, and are normally blank filled.  However, in a few cases
of binary variable length records, binary information needed to calculate the
record length is stored there.

1.1 Record types
----------------

For simplicity, and to make it easier for programmers to remember what file type
they are dealing with, the record type numbers are equal to the binary file type
number x 100 (currently 1 through 3) plus some offset.  Thus a record type 120
is immediately recognizable as belonging to a type-1 (corel) file.  The only
exception to this rule is the type-000 record, which is the first record of
every binary file regardless of file type.  The offsets are organized into
logical groupings.  Where it makes sense, all records dealing with a particular
aspect of the data are sequentially numbered.  When a new aspect of the data is
encountered, the record type jumps to the next multiple of 10.  Permissible
record type numbers are therefore:

Type-1 (corel) files:   100 to 199
Type-2 (fringe) files:  200 to 299
Type-3 (sdata) files:   300 to 399

This scheme allows ample expansion for both record and file types, is sharply
distinct from the Mk3 convention to avoid confusion, and is reasonably obvious
and logical to the programmer.

1.2 Format version number
-------------------------

The 2-digit format version number allows a mechanism for evolution of the file
format in response to unforeseen needs with minimal modifications to previously
written software.  The IO software picks up this number, and copies the record
into the memory structure appropriate to the record type and version.  Most of
the structure elements will have unchanged names from previous versions, so
software downstream of the IO library will in general not care what the version
number was.  To implement a new format version, all that must be done is to
update the header file containing the structure definitions, and write code to
process the newly-introduced fields.  Applications must include a structure
definition that incorporates a superset of all fields from all version numbers,
and the IO library must fill the fields appropriately.  In general, this
prevents the use of simple HOPS Mk3-style memory overlays into structures for
version numbers greater than 0, but the CPU overhead of explicitly filling
structure elements is minimal.  While updating the IO library for a new version
number may be non-trivial, it needs to be done in only one place, not in each
application.

The ease with which this can be done at the application level (where most of the
complexity lies) should encourage elegant and complete solutions to file-format
related difficulties, rather than counterintuitive and complicating workarounds.

The rationale for carrying a format version number with records instead of files
is that you don't want to increment a file version number for each small
modification to some obscure record.  Record version control also increases
flexibility, allowing one to transparently mix record version numbers in files,
though in practise this will seldom occur.

2. Continuation number vs. variable record lengths
--------------------------------------------------

Certain types of information have variable space requirements.  There are two
ways this might be handled.  The Mk3 system uses fixed-length 256-byte records,
and stacks records using record continuation numbers.  Under UNIX, we have the
freedom to use variable-length records with little penalty in code complexity.
After some deliberation, the decision was made to move away from continuation
numbers and toward variable-length records for Mk4.  The majority of records
nevertheless remain fixed-length simply because the amount of information needed
is invariant.

3. File identification
----------------------

**type_000**

In order to facilitate consistency checks of data files, and to ease
programmatic manipulations for filesets, it has been decided to place a special
file identification record at the beginning of each file.  This record is typed
in the same way as all other Mk4 records.  The record type is 000, and the
record length is fixed at 64 bytes.  For various reasons, it is desirable for
this record to be entirely ascii, not least because then it is trivial to
generate a greppable and user-comprehensible summary of many data files.  The
format of the type 000 record is as follows.  All fields are ascii.

+--------------+---------------+----------------+----------------------------------------------+
| Field Name   | Data Type     | Size (bytes)   | Description                                  |
+==============+===============+================+==============================================+
| record_id    | char[3]       | 3              | Standard 3-digit id                          |
+--------------+---------------+----------------+----------------------------------------------+
| version_no   | char[2]       | 2              | Standard 2-digit version #                   |
+--------------+---------------+----------------+----------------------------------------------+
| unused1      | char[3]       | 3              | Reserved space                               |
+--------------+---------------+----------------+----------------------------------------------+
| date         | char[16]      | 16             | Creation date `" yyyyddd-hhmmss "`           |
+--------------+---------------+----------------+----------------------------------------------+
| name         | char[40]      | 40             | exp/scan/name, null-terminated               |
+--------------+---------------+----------------+----------------------------------------------+

See :hops:`type_000` for more information.

4. Data alignment
-----------------

In the record definitions below, care has been taken to ensure that variables
are properly aligned.  This means that if a variable occupies n bytes, the
location of the variable within the record is an integral number times n bytes
from the start of the record.  In a few places, padding is used.  This practise,
which causes some rearrangement of variables from the order in which they might
otherwise have been stored, ensures that it will always be possible to map a
copy of the record format on disk directly onto a C structure.  In addition,
record lengths are kept to a multiple 8 bytes, to ensure that each record starts
on a suitable boundary.

Type-1 (corel) file record formats
----------------------------------

Similar to Mk3, each correlator output record corresponds to a single AP for a
single channel, with multiple lags.  The records are variable-length, with the
length determined by the number of lags present.  Each type-1 file has, as the
first record after the type 000 record, a record which contains pertinent
file-wide information, and eliminates the current "orphan" potential of the
current Mk3 system.  Mk4 correlator files are dependent on the root file for the
correct interpretation of their contents, but not for the mere identification of
the data.  The general organization of a type-1 file is as follows:

+---------------------+--------+--------------------------------------------------------------------------------------------------+
| Record Type         | Code   | Description                                                                                      |
+=====================+========+==================================================================================================+
| Type 000 record     | 000    | Standard for all binary Mk4 files                                                                |
+---------------------+--------+--------------------------------------------------------------------------------------------------+
| Type 100 record     | 100    | Identifies data by baseline, parent root, correlation time, etc.                                 |
|                     |        | Specifies how many data records are present.                                                     |
+---------------------+--------+--------------------------------------------------------------------------------------------------+
| Type 101 records    | 101    | Track-specific correlator hardware configuration information similar to the Mk3                  |
|                     |        | type-2000 record and cross-reference table.                                                      |
|                     |        | There is one type-101 record for each index number.                                              |
|                     |        | Needed for correct interpretation of the type 120 records.                                       |
+---------------------+--------+--------------------------------------------------------------------------------------------------+
| Type 120 records    | 120    | Correlation lag data records for various modes.                                                  |
|                     |        | The different modes may involve inclusion or exclusion of bitcounts by lag,                      |
|                     |        | autocorrelations, and so on. There is only one mode per file.                                    |
+---------------------+--------+--------------------------------------------------------------------------------------------------+

Because of the presence of variable length records, the type 100 record must
precede all type 101 and 120 records.  For simplicity, the number of lags (and
correlator blocks) must be constant in any given type-1 file.  There is one type
100 record, multiple type 101 records, and multiple type 120 records per type
101 record.  Some of the information in the type 100 record may not be available
until the rest of the file is written, necessitating re-writing of that record.

**type_100**  

Type 100 (general data description) record format. See :hops:`type_100` for more information.

+--------------+----------------+----------------+------------------------------------------------+
| Field Name   | Data Type      | Size (bytes)   | Description                                    |
+==============+================+================+================================================+
| record_id    | char[3]        | 3              | Standard 3-digit id                            |
+--------------+----------------+----------------+------------------------------------------------+
| version_no   | char[2]        | 2              | Standard 2-digit version #                     |
+--------------+----------------+----------------+------------------------------------------------+
| unused1      | char[3]        | 3              | Reserved space                                 |
+--------------+----------------+----------------+------------------------------------------------+
| procdate     | struct date    | 12             | Correlation time                               |
+--------------+----------------+----------------+------------------------------------------------+
| baseline     | char[2]        | 2              | Standard baseline id                           |
+--------------+----------------+----------------+------------------------------------------------+
| rootname     | char[34]       | 34             | Root filename, null-terminated                 |
+--------------+----------------+----------------+------------------------------------------------+
| qcode        | char[2]        | 2              | Quality code of correlation                    |
+--------------+----------------+----------------+------------------------------------------------+
| unused2      | char[6]        | 6              | Padding                                        |
+--------------+----------------+----------------+------------------------------------------------+
| pct_done     | float          | 4              | 0–100% of scheduled data processed             |
+--------------+----------------+----------------+------------------------------------------------+
| start        | struct date    | 12             | Time of first AP                               |
+--------------+----------------+----------------+------------------------------------------------+
| stop         | struct date    | 12             | Time of last AP                                |
+--------------+----------------+----------------+------------------------------------------------+
| ndrec        | int            | 4              | Number of data records                         |
+--------------+----------------+----------------+------------------------------------------------+
| nindex       | int            | 4              | Number of index numbers present                |
+--------------+----------------+----------------+------------------------------------------------+
| nlags        | short          | 2              | Number of lags in a type_120 record            |
+--------------+----------------+----------------+------------------------------------------------+
| nblocks      | short          | 2              | Number of blocks per index number              |
+--------------+----------------+----------------+------------------------------------------------+

Record length is fixed.  This record can be thought of as a
"consistency control" record, which should be examined to make sure that
the data are what you think they should be.  The IO library should do
most of this checking, and raise the alarm to the application programmer
when discrepancies are found.  Global hardware configuration information
is also stored here.

**type_101**  

Type 101 (index number parameter) record format. See :hops:`type_101` for more information.

+-------------+-------+-----------+-------------------------------+
| Field       | type  | bytes     | Description                   |
+=============+=======+===========+===============================+
| Type        | ascii | 3         | 101                           |
+-------------+-------+-----------+-------------------------------+
| Version     | ascii | 2         | 0-99                          |
+-------------+-------+-----------+-------------------------------+
| Status      | ascii | 1         | Currently unused, set to null |
+-------------+-------+-----------+-------------------------------+
| nblocks     | i*2   | 2         | Number of block table entries |
+-------------+-------+-----------+-------------------------------+
| Index       | i*2   | 2         | Index number                  |
+-------------+-------+-----------+-------------------------------+
| Primary     | i*2   | 2         | Index number of primary 101   |
+-------------+-------+-----------+-------------------------------+
| Ref_chan_id | ascii | 8         | from vex, e.g. X1R            |
+-------------+-------+-----------+-------------------------------+
| Rem_chan_id | ascii | 8         | from vex, e.g. X1L            |
+-------------+-------+-----------+-------------------------------+
| Corr. board | i*2   | 2         | Correlator board serial #     |
+-------------+-------+-----------+-------------------------------+
| Corr. slot  | i*2   | 2         | Correlator board slot         |
+-------------+-------+-----------+-------------------------------+
| Ref channel | i*2   | 2         | SU output channel numbers     |
+-------------+-------+-----------+-------------------------------+
| Rem channel | i*2   | 2         |                               |
+-------------+-------+-----------+-------------------------------+
| Post mortem | i*4   | 4         | Up to 32 1-bit flags          |
+-------------+-------+-----------+-------------------------------+
| Block table | i*4   | 4*nblocks | One entry per block in snake  |
+-------------+-------+-----------+-------------------------------+

Record length is variable at 40+(4*nblocks), but with a wrinkle.  In order to
maintain the record length as a multiple of 8 bytes, if nblocks is an odd
number, the block table will be padded as if nblocks were equal to the next even
number. There are i records, where i is given by (high index - low index + 1).
This record describes channel-by-channel hardware configuration information for
this correlation.  Each 'Block table' entry consists of 3 bytes of static
configuration information followed by the block number on the correlator board;
order of entries is from head of snake to tail of snake.

Some information in the type 101 records is analogous to that in the 
type-2000 records of the Mk3 system.  The Mk4 scheme helps isolate 
baseline-dependent information in the baseline-dependent type-1 files.
Type-1 files are not intended to be made standalone by this change.

**type_120**  

Type 120 (sorted lag data) record format. See :hops:`type_120` for more information.

+--------------+-------------------+----------------+------------------------------------------------+
| Field Name   | Data Type         | Size (bytes)   | Description                                    |
+==============+===================+================+================================================+
| record_id    | char[3]           | 3              | Standard 3-digit id                            |
+--------------+-------------------+----------------+------------------------------------------------+
| version_no   | char[2]           | 2              | Standard 2-digit version #                     |
+--------------+-------------------+----------------+------------------------------------------------+
| type         | char              | 1              | Data type (enumerated elsewhere)               |
+--------------+-------------------+----------------+------------------------------------------------+
| nlags        | short             | 2              | Number of lags                                 |
+--------------+-------------------+----------------+------------------------------------------------+
| baseline     | char[2]           | 2              | Standard baseline ID                           |
+--------------+-------------------+----------------+------------------------------------------------+
| rootcode     | char[6]           | 6              | Root suffix                                    |
+--------------+-------------------+----------------+------------------------------------------------+
| index        | int               | 4              | Index number for corresponding type 101 record |
+--------------+-------------------+----------------+------------------------------------------------+
| ap           | int               | 4              | Accumulation period number                     |
+--------------+-------------------+----------------+------------------------------------------------+
| fw           | union flag_wgt    | 4              | Flag or weight for lag/spectral data           |
+--------------+-------------------+----------------+------------------------------------------------+
| status       | int               | 4              | Up to 32 status bits                           |
+--------------+-------------------+----------------+------------------------------------------------+
| fr_delay     | int               | 4              | Mid-AP fractional delay (bits * 2^32)          |
+--------------+-------------------+----------------+------------------------------------------------+
| delay_rate   | int               | 4              | Mid-AP delay rate (bits/sysclk * 2^32)         |
+--------------+-------------------+----------------+------------------------------------------------+
| ld           | union lag_data    | variable       | Correlation counts (depends on data type)      |
+--------------+-------------------+----------------+------------------------------------------------+

where lagdata can have any one of five possible structures (COUNTS_PER_LAG, COUNTS_GLOBAL, AUTO_PER_LAG, AUTO_GLOBAL, or SPECTRAL). 

+----------+---------------------------+----------------+----------------------------------------------+
| Field    | Type                      | Size (bytes)   | Description                                  |
+==========+===========================+================+==============================================+
| cpl[1]   | struct counts_per_lag[1]  | variable       | Counts per lag                               |
+----------+---------------------------+----------------+----------------------------------------------+
| cg       | struct counts_global      | variable       | Global lag count data                        |
+----------+---------------------------+----------------+----------------------------------------------+
| apl[1]   | struct auto_per_lag[1]    | variable       | Auto-correlation per lag                     |
+----------+---------------------------+----------------+----------------------------------------------+
| ag       | struct auto_global        | variable       | Auto-correlation global data                 |
+----------+---------------------------+----------------+----------------------------------------------+
| spec[1]  | struct spectral[1]        | 8              | Spectral data (complex number)               |
+----------+---------------------------+----------------+----------------------------------------------+

However first four types are **DEPRECATED** and the only form which is currently used (when importing from DiFX) is the 'SPECTRAL' type, 
which has the form:

+----------+--------+----------------+-----------------------------+
| Field    | Type   | Size (bytes)   | Description                 |
+==========+========+================+=============================+
| re       | float  | 4              | Real component              |
+----------+--------+----------------+-----------------------------+
| im       | float  | 4              | Imaginary component         |
+----------+--------+----------------+-----------------------------+

for each element. The weight type for spectral data is effectively a single float, as the union has the structure:

+----------+--------+----------------+--------------------------------------------------+
| Field    | Type   | Size (bytes)   | Description                                      |
+==========+========+================+==================================================+
| flag     | int    | 4              | Up to 32 correlation flags                       |
+----------+--------+----------------+--------------------------------------------------+
| weight   | float  | 4              | Spectral mode AP weight (range 0.0 – 1.0)        |
+----------+--------+----------------+--------------------------------------------------+

Record length is variable, depending on the number of lags and the mode.
Only one mode, and therefore one lag_data format, is present in any given file.

Type-2 (fringe) file record formats
-----------------------------------

Type 2 files consist of three logically distinct sections.  The first describes
the history of the data to this point, up to and including the fringe search
numerical results.  The second contains representations of the data, rotated to
the solution parameters.  The third contains the postscript fringe plot, and
other useful graphical data.

The basic pattern is for type 200 through type 208 records to contain
information progressively less generic, and progressively more specific
to the fringe fit process that generated the type-2 file.  The progression
goes from baseline-independent information from the root in the type 200
record to the final fringe-fit solution values in the type 208 record.  Various
processed versions of the input data records are stored in record types 210 to
212, and user-oriented graphical output information is stored in records 220 and
higher.

**type_200**  

Type 200 (general information) record format. See :hops:`type_200` for more information.

+------------------+-------------------+----------------+-------------------------------------------------------------+
| Field Name       | Type              | Size (bytes)   | Description                                                 |
+==================+===================+================+=============================================================+
| record_id        | char[3]           | 3              | Standard 3-digit id                                         |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| version_no       | char[2]           | 2              | Standard 2-digit version #                                  |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| unused1          | char[3]           | 3              | Reserved space                                              |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| software_rev     | short[10]         | 20             | Revision levels for online programs                         |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| expt_no          | int               | 4              | Experiment number                                           |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| exper_name       | char[32]          | 32             | Observing program name                                      |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| scan_name        | char[32]          | 32             | Scan label from OVEX                                        |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| correlator       | char[8]           | 8              | Correlator identification                                   |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| scantime         | struct date       | 12             | Scan time to 1 second                                       |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| start_offset     | int               | 4              | Nominal baseline start relative to scantime (seconds)       |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| stop_offset      | int               | 4              | Nominal baseline stop relative to scantime (seconds)        |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| corr_date        | struct date       | 12             | Time of correlation                                         |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| fourfit_date     | struct date       | 12             | Time of fourfit processing                                  |
+------------------+-------------------+----------------+-------------------------------------------------------------+
| frt              | struct date       | 12             | Fourfit reference time                                      |
+------------------+-------------------+----------------+-------------------------------------------------------------+

Record length is fixed.

**type_201**

Type 201 (source information) record format. See :hops:`type_201` for more information. 

.. note::
   The dispersion measure in this structure is used for storing the ionospheric differential TEC for the associated baseline.

+------------------+------------------------+----------------+----------------------------------------------------------+
| Field Name       | Type                   | Size (bytes)   | Description                                              |
+==================+========================+================+==========================================================+
| record_id        | char[3]                | 3              | Standard 3-digit id                                      |
+------------------+------------------------+----------------+----------------------------------------------------------+
| version_no       | char[2]                | 2              | Standard 2-digit version #                               |
+------------------+------------------------+----------------+----------------------------------------------------------+
| unused1          | char[3]                | 3              | Reserved space                                           |
+------------------+------------------------+----------------+----------------------------------------------------------+
| source           | char[32]               | 32             | Source name from OVEX                                    |
+------------------+------------------------+----------------+----------------------------------------------------------+
| coord            | struct sky_coord       | 16             | Source coordinates                                       |
+------------------+------------------------+----------------+----------------------------------------------------------+
| epoch            | short                  | 2              | Coordinate epoch (e.g., 1950 or 2000)                    |
+------------------+------------------------+----------------+----------------------------------------------------------+
| unused2          | char[2]                | 2              | Padding                                                  |
+------------------+------------------------+----------------+----------------------------------------------------------+
| coord_date       | struct date            | 12             | Date of coordinate measurement                           |
+------------------+------------------------+----------------+----------------------------------------------------------+
| ra_rate          | double                 | 8              | Proper motion in right ascension (rad/sec)               |
+------------------+------------------------+----------------+----------------------------------------------------------+
| dec_rate         | double                 | 8              | Proper motion in declination (rad/sec)                   |
+------------------+------------------------+----------------+----------------------------------------------------------+
| pulsar_phase     | double[4]              | 32             | Polynomial coefficients for pulsar timing                |
+------------------+------------------------+----------------+----------------------------------------------------------+
| pulsar_epoch     | double                 | 8              | Reference time for pulsar timing polynomial              |
+------------------+------------------------+----------------+----------------------------------------------------------+
| dispersion       | double                 | 8              | dispersion                                               |
+------------------+------------------------+----------------+----------------------------------------------------------+

Record length is fixed.  This record contains source-specific information.


**type_202**

Type 202 (baseline information) record format. See :hops:`type_202` for more information.

+-------------------+-------------------+----------------+---------------------------------------------------------+
| Field Name        | Type              | Size (bytes)   | Description                                             |
+===================+===================+================+=========================================================+
| record_id         | char[3]           | 3              | Standard 3-digit id                                     |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| version_no        | char[2]           | 2              | Standard 2-digit version #                              |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| unused1           | char[3]           | 3              | Reserved space                                          |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| baseline          | char[2]           | 2              | 2-char baseline ID                                      |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_intl_id       | char[2]           | 2              | Reference station international ID                      |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_intl_id       | char[2]           | 2              | Remote station international ID                         |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_name          | char[8]           | 8              | Reference station name                                  |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_name          | char[8]           | 8              | Remote station name                                     |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_tape          | char[8]           | 8              | Reference station tape VSN                              |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_tape          | char[8]           | 8              | Remote station tape VSN                                 |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| nlags             | short             | 2              | Number of lags used for correlation                     |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_xpos          | double            | 8              | Reference station X coordinate (meters)                 |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_xpos          | double            | 8              | Remote station X coordinate (meters)                    |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_ypos          | double            | 8              | Reference station Y coordinate (meters)                 |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_ypos          | double            | 8              | Remote station Y coordinate (meters)                    |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_zpos          | double            | 8              | Reference station Z coordinate (meters)                 |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_zpos          | double            | 8              | Remote station Z coordinate (meters)                    |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| u                 | double            | 8              | Fringes/arcsec E-W at 1 GHz                             |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| v                 | double            | 8              | Fringes/arcsec N-S at 1 GHz                             |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| uf                | double            | 8              | mHz/arcsec/GHz in Right Ascension                       |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| vf                | double            | 8              | mHz/arcsec/GHz in Declination                           |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_clock         | float             | 4              | Reference station clock (μsec)                          |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_clock         | float             | 4              | Remote station clock (μsec)                             |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_clockrate     | float             | 4              | Reference station clock rate (sec/sec)                  |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_clockrate     | float             | 4              | Remote station clock rate (sec/sec)                     |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_idelay        | float             | 4              | Reference station instrumental delay (μsec)             |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_idelay        | float             | 4              | Remote station instrumental delay (μsec)                |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_zdelay        | float             | 4              | Reference station zenith atmospheric delay (μsec)       |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_zdelay        | float             | 4              | Remote station zenith atmospheric delay (μsec)          |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_elev          | float             | 4              | Elevation at reference antenna (degrees)                |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_elev          | float             | 4              | Elevation at remote antenna (degrees)                   |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| ref_az            | float             | 4              | Azimuth at reference antenna (degrees)                  |
+-------------------+-------------------+----------------+---------------------------------------------------------+
| rem_az            | float             | 4              | Azimuth at remote antenna (degrees)                     |
+-------------------+-------------------+----------------+---------------------------------------------------------+

This record contains baseline specific information, independent of fourfit parameters except FRT.

**type_203**

Type 203 (channel information) record format. See :hops:`type_203` for more information.

.. note::
   The value of MAXFREQ is 64.


+-------------------+------------------------+----------------+-----------------------------------------------------------+
| Field Name        | Type                   | Size (bytes)   | Description                                               |
+===================+========================+================+===========================================================+
| record_id         | char[3]                | 3              | Standard 3-digit id                                       |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| version_no        | char[2]                | 2              | Standard 2-digit version #                                |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| unused1           | char[3]                | 3              | Reserved space                                            |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| channels          | ch_struct[8*MAXFREQ]   |                | Array of 512 channel info structs (8 per frequency × 64)  |
|                   |                        |                |                                                           |
+-------------------+------------------------+----------------+-----------------------------------------------------------+

Details of `struct ch_struct`:

+-------------------+------------------------+----------------+-----------------------------------------------------------+
| Field Name        | Type                   | Size (bytes)   | Description                                               |
+===================+========================+================+===========================================================+
| index             | short                  | 2              | Index from type-1 file (t101)                             |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| sample_rate       | unsigned short int     | 2              | Sample rate in Ksamp/sec (max 65.536 MSamp/s)             |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| refsb             | char                   | 1              | Reference antenna sideband ('U' or 'L')                   |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| remsb             | char                   | 1              | Remote antenna sideband ('U' or 'L')                      |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| refpol            | char                   | 1              | Reference antenna polarization ('R' or 'L')               |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| rempol            | char                   | 1              | Remote antenna polarization ('R' or 'L')                  |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| ref_freq          | double                 | 8              | Sky frequency at reference station (MHz)                  |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| rem_freq          | double                 | 8              | Sky frequency at remote station (MHz)                     |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| ref_chan_id       | char[8]                | 8              | Reference station channel ID                              |
+-------------------+------------------------+----------------+-----------------------------------------------------------+
| rem_chan_id       | char[8]                | 8              | Remote station channel ID                                 |
+-------------------+------------------------+----------------+-----------------------------------------------------------+

Record length is fixed. This is a copy of the information in all
the (non-mirrored) type 101 records in the type-1 file, regardless of whether
the channels are used in the current fringe-fit.  A "channel" described by a
single entry in the array corresponds to the cross-correlation of a single pair
of channel IDs as described in the $FREQ section of the ovex portion of the root
file.  Note that a fourfit frequency channel may consist of multiple such
"channels", depending on whether this is dual-sideband data, or possibly even
dual polarization data in which RR and LL correlations have been combined before
the fringe fit.

**type_204**

Type 204 (execution setup) record format. See :hops:`type_204` for more information.

+-------------------+--------------------+----------------+----------------------------------------------------+
| Field Name        | Type               | Size (bytes)   | Description                                        |
+===================+====================+================+====================================================+
| record_id         | char[3]            | 3              | Standard 3-digit id                                |
+-------------------+--------------------+----------------+----------------------------------------------------+
| version_no        | char[2]            | 2              | Standard 2-digit version #                         |
+-------------------+--------------------+----------------+----------------------------------------------------+
| unused1           | char[3]            | 3              | Reserved space                                     |
+-------------------+--------------------+----------------+----------------------------------------------------+
| ff_version        | short[2]           | 4              | Fourfit revision level                             |
+-------------------+--------------------+----------------+----------------------------------------------------+
| platform          | char[8]            | 8              | Platform string (e.g., hppa, linux, alpha)         |
+-------------------+--------------------+----------------+----------------------------------------------------+
| control_file      | char[96]           | 96             | Full pathname of the control file                  |
+-------------------+--------------------+----------------+----------------------------------------------------+
| ffcf_date         | struct date        | varies         | Control file modification date                     |
+-------------------+--------------------+----------------+----------------------------------------------------+
| override          | char[128]          | 128            | Command-line override string                       |
+-------------------+--------------------+----------------+----------------------------------------------------+

Record length is fixed. The strings are null-terminated.  If
they overflow, the strings are set to null.  This record is just a tracer
of the execution parameters for possible subsequent human intervention.

**type_205**

Type 205 (fourfit setup) record format. See :hops:`type_205` for more information.

+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| Field Name               | Type                      | Size (bytes)   | Description                                                 |
+==========================+===========================+================+=============================================================+
| record_id                | char[3]                   | 3              | Standard 3-digit id                                         |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| version_no               | char[2]                   | 2              | Standard 2-digit version #                                  |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| unused1                  | char[3]                   | 3              | Reserved space                                              |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| utc_central              | struct date               | 12             | Central time of scan                                        |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| offset                   | float                     | 4              | Offset of FRT from scan center (seconds)                    |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| ffmode                   | char[8]                   | 8              | Fourfit execution modes                                     |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| search                   | float[6]                  | 24             | SBD, MBD, rate search windows (usec, usec, usec/sec)        |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| filter                   | float[8]                  | 32             | Various filter thresholds                                   |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| start                    | struct date               | 12             | Start of requested data span                                |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| stop                     | struct date               | 12             | End of requested data span                                  |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| ref_freq                 | double                    | 8              | Fourfit reference frequency (Hz)                            |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+
| ffit_chan                | struct ffit_chan[64]      |                | Array of Fourfit channel ID info structs                    |
+--------------------------+---------------------------+----------------+-------------------------------------------------------------+

The ffit_chan structures have the following form:

+--------------------+-------------+----------------+---------------------------------------------------+
| Field Name         | Type        | Size (bytes)   | Description                                       |
+====================+=============+================+===================================================+
| ffit_chan_id       | char        | 1              | Fourfit channel letter ID                         |
+--------------------+-------------+----------------+---------------------------------------------------+
| unused             | char        | 1              | Alignment padding                                 |
+--------------------+-------------+----------------+---------------------------------------------------+
| channels           | short[4]    | 8              | Indices into `type_203` channel array             |
+--------------------+-------------+----------------+---------------------------------------------------+

This record describes the setup of the fourfit execution, independent of the AP data.

**type_206**

Type 206 (data filtering) record format. See :hops:`type_206` for more information.

+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| Field Name          | Type                        | Size (bytes)   | Description                                                  |
+=====================+=============================+================+==============================================================+
| record_id           | char[3]                     | 3              | Standard 3-digit ID                                          |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| version_no          | char[2]                     | 2              | Standard 2-digit version number                              |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| unused1             | char[3]                     | 3              | Reserved space                                               |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| start               | struct date                 | 12             | Time at start of AP zero                                     |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| first_ap            | short                       | 2              | Number of first valid accumulation period                    |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| last_ap             | short                       | 2              | Number of last valid accumulation period                     |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| accepted            | struct sidebands[64]        | 256            | APs accepted by channel/sideband                             |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| weights             | struct sbweights[64]        | 1024           | Samples per channel/sideband                                 |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| intg_time           | float                       | 4              | Effective integration time in seconds                        |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| accept_ratio        | float                       | 4              | Percentage ratio of min/max data accepted                    |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| discard             | float                       | 4              | Percentage of data discarded                                 |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| reason1             | struct sidebands[64]        | 256            | APs filtered out by reason 1 per chan/sband                  |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| reason2             | struct sidebands[64]        | 256            | APs filtered out by reason 2 per chan/sband                  |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| reason3             | struct sidebands[64]        | 256            | APs filtered out by reason 3 per chan/sband                  |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| reason4             | struct sidebands[64]        | 256            | APs filtered out by reason 4 per chan/sband                  |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| reason5             | struct sidebands[64]        | 256            | APs filtered out by reason 5 per chan/sband                  |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| reason6             | struct sidebands[64]        | 256            | APs filtered out by reason 6 per chan/sband                  |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| reason7             | struct sidebands[64]        | 256            | APs filtered out by reason 7 per chan/sband                  |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| reason8             | struct sidebands[64]        | 256            | APs filtered out by reason 8 per chan/sband                  |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| ratesize            | short                       | 2              | Size of fringe rate transform                                |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| mbdsize             | short                       | 2              | Size of multi-band delay (MBD) transform                     |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| sbdsize             | short                       | 2              | Size of single-band delay (SBD) transform                    |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+
| unused2             | char[6]                     | 6              | Reserved/padding space                                       |
+---------------------+-----------------------------+----------------+--------------------------------------------------------------+

The sideband struct has the form:

+------------+--------+----------------+----------------------------------------------+
| Field      | Type   | Size (bytes)   | Description                                  |
+============+========+================+==============================================+
| lsb        | short  | 2              | Lower sideband value                         |
+------------+--------+----------------+----------------------------------------------+
| usb        | short  | 2              | Upper sideband value                         |
+------------+--------+----------------+----------------------------------------------+

while the sbweights struct has the form:

+------------+--------+----------------+----------------------------------------------+
| Field      | Type   | Size (bytes)   | Description                                  |
+============+========+================+==============================================+
| lsb        | double | 8              | Lower sideband sample weight                 |
+------------+--------+----------------+----------------------------------------------+
| usb        | double | 8              | Upper sideband sample weight                 |
+------------+--------+----------------+----------------------------------------------+

This record describes the data filtering action taken by fourfit in detail.
The AP flagging criteria for Mk4 will undoubtedly be different, but this is a start.  Info is
much more detailed than Mk3, since records are kept by channel/sband.
The record also describes the array sizes that fourfit decided to use.

**type_207**

Type 207 (phasecal and error rate) record format. See :hops:`type_207` for more information.

+-------------------+------------------------+----------------+-------------------------------------------------------------+
| Field Name        | Type                   | Size (bytes)   | Description                                                 |
+===================+========================+================+=============================================================+
| record_id         | char[3]                | 3              | Standard 3-digit ID                                         |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| version_no        | char[2]                | 2              | Standard 2-digit version number                             |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| unused1           | char[3]                | 3              | Reserved space                                              |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| pcal_mode         | int                    | 4              | 10 × ant1 + ant2; meaning defined in `control.h`            |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| unused2           | int                    | 4              | Padding                                                     |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| ref_pcamp         | sbandf[64]             | 512            | Phasecal amplitude for reference station                    |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| rem_pcamp         | sbandf[64]             | 512            | Phasecal amplitude for remote station                       |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| ref_pcphase       | sbandf[64]             | 512            | Phasecal phase for reference station                        |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| rem_pcphase       | sbandf[64]             | 512            | Phasecal phase for remote station                           |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| ref_pcoffset      | sbandf[64]             | 512            | Phasecal offset for reference station                       |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| rem_pcoffset      | sbandf[64]             | 512            | Phasecal offset for remote station                          |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| ref_pcfreq        | sbandf[64]             | 512            | Phasecal frequency for reference station                    |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| rem_pcfreq        | sbandf[64]             | 512            | Phasecal frequency for remote station                       |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| ref_pcrate        | float                  | 4              | Phasecal rate for reference station                         |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| rem_pcrate        | float                  | 4              | Phasecal rate for remote station                            |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| ref_errate        | float[64]              | 256            | Mean error rate per tone for reference station              |
+-------------------+------------------------+----------------+-------------------------------------------------------------+
| rem_errate        | float[64]              | 256            | Mean error rate per tone for remote station                 |
+-------------------+------------------------+----------------+-------------------------------------------------------------+

the sbandf structure has the form:

+--------+--------+----------------+-------------------------------------------+
| Field  | Type   | Size (bytes)   | Description                               |
+========+========+================+===========================================+
| lsb    | float  | 4              | Value for lower sideband                  |
+--------+--------+----------------+-------------------------------------------+
| usb    | float  | 4              | Value for upper sideband                  |
+--------+--------+----------------+-------------------------------------------+

The phasecal and errorate numbers are as used by fourfit after extraction from the type-3 files,
and interpretation in terms of the fourfit input parameters.  Error rates
are by channel, translated from track error rates by arithmetic average
of all contributing tracks.

**type_208**

Type 208 (solution parameter) record format. See :hops:`type_208` for more information.

+---------------------+-------------+----------------+--------------------------------------------------------------+
| Field Name          | Data Type   | Size (bytes)   | Description                                                  |
+=====================+=============+================+==============================================================+
| record_id           | char[3]     | 3              | Standard 3-digit id                                          |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| version_no          | char[2]     | 2              | Standard 2-digit version #                                   |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| unused1             | char[3]     | 3              | Reserved space                                               |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| quality             | char        | 1              | Fringe quality 0 to 9                                        |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| errcode             | char        | 1              | A to F, maybe others                                         |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| tape_qcode          | char[6]     | 6              | For A-file backward compat.                                  |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| adelay              | double      | 8              | Apriori delay at FRT (usec)                                  |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| arate               | double      | 8              | Apriori rate at FRT (usec/sec)                               |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| aaccel              | double      | 8              | Apriori accel at FRT (usec/sec²)                             |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| tot_mbd             | double      | 8              | Total observed MBD (usec)                                    |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| tot_sbd             | double      | 8              | Total observed SBD (usec)                                    |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| tot_rate            | double      | 8              | Total observed rate (usec/sec)                               |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| tot_mbd_ref         | double      | 8              | Total observed MBD (usec) at ref stn epoch                   |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| tot_sbd_ref         | double      | 8              | Total observed SBD (usec) at ref stn epoch                   |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| tot_rate_ref        | double      | 8              | Total observed rate (usec/sec) at ref stn epoch              |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| resid_mbd           | float       | 4              | MBD residual to model (usec)                                 |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| resid_sbd           | float       | 4              | SBD residual to model (usec)                                 |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| resid_rate          | float       | 4              | Rate residual to model (usec/sec)                            |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| mbd_error           | float       | 4              | MBD error calculated from data (usec)                        |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| sbd_error           | float       | 4              | SBD error calculated from data (usec)                        |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| rate_error          | float       | 4              | Rate error calculated from data (usec/sec)                   |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| ambiguity           | float       | 4              | MBD ambiguity (usec)                                         |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| amplitude           | float       | 4              | Coherent amplitude (correlation coefficient)                 |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| inc_seg_ampl        | float       | 4              | Incoherent segment addition amplitude                        |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| inc_chan_ampl       | float       | 4              | Incoherent channel addition amplitude                        |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| snr                 | float       | 4              | SNR in sigmas                                                |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| prob_false          | float       | 4              | Probability of false detection                               |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| totphase            | float       | 4              | Total observed fringe phase (deg)                            |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| totphase_ref        | float       | 4              | Total phase at ref station epoch                             |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| resphase            | float       | 4              | Residual earth-centered phase (deg)                          |
+---------------------+-------------+----------------+--------------------------------------------------------------+
| tec_error           | float       | 4              | Std dev of TEC estimate (TEC units)                          |
+---------------------+-------------+----------------+--------------------------------------------------------------+

Record length is fixed. This record contains the essentials of the fringe fit solution.

**type_210**

Type 210 (channel data) record format. See :hops:`type_210` for more information.

+----------------+--------------------+----------------+-------------------------------------------+
| Field Name     | Data Type          | Size (bytes)   | Description                               |
+================+====================+================+===========================================+
| record_id      | char[3]            | 3              | Standard 3-digit id                       |
+----------------+--------------------+----------------+-------------------------------------------+
| version_no     | char[2]            | 2              | Standard 2-digit version #                |
+----------------+--------------------+----------------+-------------------------------------------+
| unused1        | char[3]            | 3              | Reserved space                            |
+----------------+--------------------+----------------+-------------------------------------------+
| amp_phas       | struct polars[64]  | variable       | Residual fringe amp/phase                 |
+----------------+--------------------+----------------+-------------------------------------------+

Record length is fixed. The entries are equivalent to the entries
in the Mk3 type-4500 record.

**type_212** 

Type 212 (AP data) record format. See :hops:`type_212` for more information.

+----------------+-------------------+----------------+-------------------------------------------+
| Field Name     | Data Type         | Size (bytes)   | Description                               |
+================+===================+================+===========================================+
| record_id      | char[3]           | 3              | Standard 3-digit id                       |
+----------------+-------------------+----------------+-------------------------------------------+
| version_no     | char[2]           | 2              | Standard 2-digit version #                |
+----------------+-------------------+----------------+-------------------------------------------+
| unused         | char              | 1              | Unused                                    |
+----------------+-------------------+----------------+-------------------------------------------+
| nap            | short             | 2              | Needed by IO library                      |
+----------------+-------------------+----------------+-------------------------------------------+
| first_ap       | short             | 2              | Number of first ap in record              |
+----------------+-------------------+----------------+-------------------------------------------+
| channel        | short             | 2              | fourfit channel number                    |
+----------------+-------------------+----------------+-------------------------------------------+
| sbd_chan       | short             | 2              | Singleband delay channel                  |
+----------------+-------------------+----------------+-------------------------------------------+
| unused2        | char[2]           | 2              | Unused                                    |
+----------------+-------------------+----------------+-------------------------------------------+
| data           | struct newphasor* | variable       | Data values, variable length array        |
+----------------+-------------------+----------------+-------------------------------------------+


The number a is the number of APs in the processed data span.  Record length 
is variable.  There are multiple type-212 records.  
The data are rotated to the fourfit solution, unless additional SBD channels 
are dumped in which case those records are rotated to the center of the SBD 
channel in question.

**type_220**

Type 220 (fringe plot) record format. This type is **DEPRECATED**
See :hops:`type_220` for more information.

+-------------+-------+-------+----------------------------+
| Field       | type  | bytes | Description                |
+=============+=======+=======+============================+
| Type        | ascii | 3     | 220                        |
+-------------+-------+-------+----------------------------+
| Version     | ascii | 2     | 0-99                       |
+-------------+-------+-------+----------------------------+
| Unused      | ascii | 3     | Spaces                     |
+-------------+-------+-------+----------------------------+
| Fringe plot | ascii | 15360 | Standard fringe plot image |
+-------------+-------+-------+----------------------------+

**type_221**

Type 221 (postscript plot) record format. See :hops:`type_221` for more information.

+-------------+-------------+--------------+-----------------------------------------------+
| Field       | Type        | Size (bytes) | Description                                   |
+=============+=============+==============+===============================================+
| record_id   | char[3]     | 3            | Standard 3-digit ID                           |
+-------------+-------------+--------------+-----------------------------------------------+
| version_no  | char[2]     | 2            | Standard 2-digit version number               |
+-------------+-------------+--------------+-----------------------------------------------+
| unused1    | char         | 1            | Reserved space                                |
+-------------+-------------+--------------+-----------------------------------------------+
| padded      | short       | 2            | Flag for padding to 8-byte boundary           |
+-------------+-------------+--------------+-----------------------------------------------+
| ps_length   | int         | 4            | Size of postscript plot in characters         |
+-------------+-------------+--------------+-----------------------------------------------+
| pplot       | char[1]     | ps_length    | Postscript data (variable length)             |
+-------------+-------------+--------------+-----------------------------------------------+

**type_222**  

This record stores the parsed control file, for future reference. See :hops:`type_222` for more information.

+---------------------+--------------+--------------+------------------------------------------------------+
| Field               | Type         | Size (bytes) | Description                                          |
+=====================+==============+==============+======================================================+
| record_id           | char[3]      | 3            | Standard 3-digit ID                                  |
+---------------------+--------------+--------------+------------------------------------------------------+
| version_no          | char[2]      | 2            | Standard 2-digit version number                      |
+---------------------+--------------+--------------+------------------------------------------------------+
| unused1             | char         | 1            | Reserved space                                       |
+---------------------+--------------+--------------+------------------------------------------------------+
| padded              | short        | 2            | Flag for padding to 8-byte boundary                  |
+---------------------+--------------+--------------+------------------------------------------------------+
| setstring_hash      | unsigned int | 4            | Hash of setstring contents                           |
+---------------------+--------------+--------------+------------------------------------------------------+
| control_hash        | unsigned int | 4            | Hash of control file contents                        |
+---------------------+--------------+--------------+------------------------------------------------------+
| setstring_length    | int          | 4            | Size of set string in characters                     |
+---------------------+--------------+--------------+------------------------------------------------------+
| cf_length           | int          | 4            | Size of control file in characters                   |
+---------------------+--------------+--------------+------------------------------------------------------+
| control_contents    | char[8]      | varies       | Variable length array containing set string and      |
|                     |              |              | control file contents, padded to multiple of 8 bytes |
+---------------------+--------------+--------------+------------------------------------------------------+

The set-string and control file hash are computed as a simple Adler-32 checksum of the parsed byte stream.

**type_230**  

The type_230 is useful for the export of the raw fringe fit visibility data to other downstream software. It is 
an optional record which is only inserted if the ``'-X'`` option is passed to ``fourfit``.

+-------------+------------------+--------------+---------------------------------------------+
| Field       | Type             | Size (bytes) | Description                                 |
+=============+==================+==============+=============================================+
| record_id   | char[3]          | 3            | Standard 3-digit id                         |
+-------------+------------------+--------------+---------------------------------------------+
| version_no  | char[2]          | 2            | Standard 2-digit version number             |
+-------------+------------------+--------------+---------------------------------------------+
| unused1     | char             | 1            | Reserved space                              |
+-------------+------------------+--------------+---------------------------------------------+
| nspec_pts   | short            | 2            | Number of spectral points (needed by IO)    |
+-------------+------------------+--------------+---------------------------------------------+
| frq         | int              | 4            | Index into type 205                         |
+-------------+------------------+--------------+---------------------------------------------+
| ap          | int              | 4            | AP number (reference to type 206)           |
+-------------+------------------+--------------+---------------------------------------------+
| lsbweight   | float            | 4            | Fraction of AP represented in LSB           |
+-------------+------------------+--------------+---------------------------------------------+
| usbweight   | float            | 4            | Fraction of AP represented in USB           |
+-------------+------------------+--------------+---------------------------------------------+
| xpower      | hops_scomplex*   | varies       | Array of spectrum values                    |
+-------------+------------------+--------------+---------------------------------------------+

The struct hops_scomplex has the form: 

+------------+--------+----------------+----------------------------------------------+
| Field      | Type   | Size (bytes)   | Description                                  |
+============+========+================+==============================================+
| real       | double | 8              | real value                                   |
+------------+--------+----------------+----------------------------------------------+
| imag       | double | 8              | imaginary value                              |
+------------+--------+----------------+----------------------------------------------+

See :hops:`type_230` for more information.

Type 3 (Station unit data) file record formats
----------------------------------------------

The type 3 files contain station-specific information for each of the
stations involved in the scan.  They are initially created by genaroot, which
writes the type 300 and 301/2 records.  These contain the delay and phase spline
polynomials which describe the correlator model, and which drive the station
unit in its delay compensation and frame header construction.  The files are
appended to by the station unit manager software during or after the
correlation, with information on track error rates, state counts, and extracted
phasecal values in type 304, 306 and 308 records respectively.

**type_300**  

Type 300 (station ID and model parameter) record format. See :hops:`type_300` for more information.

+----------------+-------+-------+-----------------------------------+
| Field          | type  | bytes | Description                       |
+================+=======+=======+===================================+
| Type           | ascii | 3     | 300                               |
+----------------+-------+-------+-----------------------------------+
| Version        | ascii | 2     | 0-99                              |
+----------------+-------+-------+-----------------------------------+
| Unused         | ascii | 3     | Spaces                            |
+----------------+-------+-------+-----------------------------------+
| Id             | ascii | 1     | 1-char vex letter code            |
+----------------+-------+-------+-----------------------------------+
| Intl_id        | ascii | 2     | 2-char international station code |
+----------------+-------+-------+-----------------------------------+
| Name           | ascii | 32    | Full station name                 |
+----------------+-------+-------+-----------------------------------+
| Unused         | ascii | 1     | Padding for alignment             |
+----------------+-------+-------+-----------------------------------+
| Model_date     | date  | 12    | Start time for 1st spline         |
+----------------+-------+-------+-----------------------------------+
| Model interval | r*4   | 4     | Spline interval in seconds        |
+----------------+-------+-------+-----------------------------------+
| Nsplines       | i*2   | 2     | Number of splines in scan         |
+----------------+-------+-------+-----------------------------------+

Record length is fixed.

**type_301**  

Type 301 (delay polynomial coefficient) record format. See :hops:`type_301` for more information.

+--------------+---------+-------+----------------------------------+
| Field        | type    | bytes | Description                      |
+==============+=========+=======+==================================+
| Type         | ascii   | 3     | 301                              |
+--------------+---------+-------+----------------------------------+
| Version      | ascii   | 2     | 0-99                             |
+--------------+---------+-------+----------------------------------+
| Unused       | ascii   | 3     | Spaces                           |
+--------------+---------+-------+----------------------------------+
| Interval     | i*2     | 2     | Sequential model interval number |
+--------------+---------+-------+----------------------------------+
| Chan_id      | ascii   | 32    | Frequency channel identifier     |
+--------------+---------+-------+----------------------------------+
| Unused       | ascii   | 6     | Padding for alignment            |
+--------------+---------+-------+----------------------------------+
| Delay_spline | r*8 x 6 | 48    | Delay spline coefficients        |
+--------------+---------+-------+----------------------------------+

Record length is fixed.

**type_302**  

Type 302 (phase polynomial coefficient) record format. See :hops:`type_302` for more information.

+--------------+---------+-------+----------------------------------+
| Field        | type    | bytes | Description                      |
+==============+=========+=======+==================================+
| Type         | ascii   | 3     | 302                              |
+--------------+---------+-------+----------------------------------+
| Version      | ascii   | 2     | 0-99                             |
+--------------+---------+-------+----------------------------------+
| Unused       | ascii   | 3     | Spaces                           |
+--------------+---------+-------+----------------------------------+
| Interval     | i*2     | 2     | Sequential model interval number |
+--------------+---------+-------+----------------------------------+
| Chan_id      | ascii   | 32    | Frequency channel identifier     |
+--------------+---------+-------+----------------------------------+
| Unused       | ascii   | 6     | Padding for alignment            |
+--------------+---------+-------+----------------------------------+
| Phase_spline | r*8 x 6 | 48    | Phase spline coefficients        |
+--------------+---------+-------+----------------------------------+

Record length is fixed.

**type_303**  

Type 303 ("raw" track error statistics) record format. See :hops:`type_303` for more information.

The ``type_303`` struct contains the spline coefficients of the a priori model for each channel of the respective station
for the following coordinate quantities:

  - azimuth 
  - elevation 
  - parallactic_angle
  - u coordinate 
  - v coordinate 
  - w coordinate

A polynomial spline module with up to 6 coefficients is supported.

+------------------------+------------+----------------+-------------------------------------------------------------+
| Field Name             | Data Type  | Size (bytes)   | Description                                                 |
+========================+============+================+=============================================================+
| record_id              | char[3]    | 3              | Standard 3-digit id                                         |
+------------------------+------------+----------------+-------------------------------------------------------------+
| version_no             | char[2]    | 2              | Standard 2-digit version #                                  |
+------------------------+------------+----------------+-------------------------------------------------------------+
| unused1                | char[3]    | 3              | Reserved space                                              |
+------------------------+------------+----------------+-------------------------------------------------------------+
| interval               | short      | 2              | Sequential model interval number                            |
+------------------------+------------+----------------+-------------------------------------------------------------+
| chan_id                | char[32]   | 32             | Frequency channel identifier                                |
+------------------------+------------+----------------+-------------------------------------------------------------+
| unused2                | char[6]    | 6              | Padding                                                     |
+------------------------+------------+----------------+-------------------------------------------------------------+
| azimuth                | double[6]  | 48             | Azimuth (deg) coefficients                                  |
+------------------------+------------+----------------+-------------------------------------------------------------+
| elevation              | double[6]  | 48             | Elevation (deg) coefficients                                |
+------------------------+------------+----------------+-------------------------------------------------------------+
| parallactic_angle      | double[6]  | 48             | Parallactic angle (deg CCW el line from RA line)            |
+------------------------+------------+----------------+-------------------------------------------------------------+
| u                      | double[6]  | 48             | Baseline projections toward source (m)                      |
+------------------------+------------+----------------+-------------------------------------------------------------+
| v                      | double[6]  | 48             |                                                             |
+------------------------+------------+----------------+-------------------------------------------------------------+
| w                      | double[6]  | 48             |                                                             |
+------------------------+------------+----------------+-------------------------------------------------------------+


**type_304**  

Type 304 ("cooked" track error statistics) record format. See :hops:`type_304` for more information.

+------------------+-------+-------+---------------------------------------+
| Field            | type  | bytes | Description                           |
+==================+=======+=======+=======================================+
| Type             | ascii | 3     | 304                                   |
+------------------+-------+-------+---------------------------------------+
| Version          | ascii | 2     | 0-99                                  |
+------------------+-------+-------+---------------------------------------+
| Unused           | ascii | 3     | Spaces                                |
+------------------+-------+-------+---------------------------------------+
| Time             | date  | 12    | Start time of current error stats     |
+------------------+-------+-------+---------------------------------------+
| Duration         | r*4   | 4     | Duration of current error stats (sec) |
+------------------+-------+-------+---------------------------------------+
| Statistics x 64  |       |       |                                       |
+------------------+-------+-------+---------------------------------------+
| Error_rate       | r*4   | 4     | Fraction                              |
+------------------+-------+-------+---------------------------------------+
| Frames           | i*4   | 4     | Count                                 |
+------------------+-------+-------+---------------------------------------+
| Bad_frames       | i*4   | 4     | Count                                 |
+------------------+-------+-------+---------------------------------------+
| Slip_sync        | i*4   | 4     | Count                                 |
+------------------+-------+-------+---------------------------------------+
| Missing_sync     | i*4   | 4     | Count                                 |
+------------------+-------+-------+---------------------------------------+
| CRC_error        | i*4   | 4     | Count                                 |
+------------------+-------+-------+---------------------------------------+

Record length is fixed.

**type_305**  

Type 305 ("raw" state count) record format. See :hops:`type_305` for more information.

+---------+-------+-------+-------------+
| Field   | type  | bytes | Description |
+=========+=======+=======+=============+
| Type    | ascii | 3     | 305         |
+---------+-------+-------+-------------+
| Version | ascii | 2     | 0-99        |
+---------+-------+-------+-------------+
| Unused  | ascii | 3     | Spaces      |
+---------+-------+-------+-------------+

**type_306**  

Type 306 ("cooked" state count) record format. See :hops:`type_306` for more information.

+--------------+-------+-------+-----------------------------------------+
| Field        | type  | bytes | Description                             |
+==============+=======+=======+=========================================+
| Type         | ascii | 3     | 306                                     |
+--------------+-------+-------+-----------------------------------------+
| Version      | ascii | 2     | 0-99                                    |
+--------------+-------+-------+-----------------------------------------+
| Unused       | ascii | 3     | Spaces                                  |
+--------------+-------+-------+-----------------------------------------+
| Time         | date  | 12    | Start time of current counts            |
+--------------+-------+-------+-----------------------------------------+
| Duration     | r*4   | 4     | Duration of current counts (sec)        |
+--------------+-------+-------+-----------------------------------------+
| Stcount x 16 |       |       |                                         |
+--------------+-------+-------+-----------------------------------------+
| Chan_id      | ascii | 32    | Frequency channel identifier            |
+--------------+-------+-------+-----------------------------------------+
| Bigpos       | i*4   | 4     | Count of big positive voltage samples   |
+--------------+-------+-------+-----------------------------------------+
| Pos          | i*4   | 4     | Count of small positive voltage samples |
+--------------+-------+-------+-----------------------------------------+
| Neg          | i*4   | 4     | Count of small negative voltage samples |
+--------------+-------+-------+-----------------------------------------+
| Bigneg       | i*4   | 4     | Count of big negative voltage samples   |
+--------------+-------+-------+-----------------------------------------+

Record length is fixed. 

**type_307** 

Type 307 ("raw" phase cal value) record format. See :hops:`type_307` for more information.

+---------+-------+-------+-------------+
| Field   | type  | bytes | Description |
+=========+=======+=======+=============+
| Type    | ascii | 3     | 307         |
+---------+-------+-------+-------------+
| Version | ascii | 2     | 0-99        |
+---------+-------+-------+-------------+
| Unused  | ascii | 3     | Spaces      |
+---------+-------+-------+-------------+


**type_308**  

Type 308 ("cooked" phase cal value) record format. See :hops:`type_308` for more information.

+-----------+-------+-------+----------------------------------+
| Field     | type  | bytes | Description                      |
+===========+=======+=======+==================================+
| Type      | ascii | 3     | 308                              |
+-----------+-------+-------+----------------------------------+
| Version   | ascii | 2     | 0-99                             |
+-----------+-------+-------+----------------------------------+
| Unused    | ascii | 3     | Spaces                           |
+-----------+-------+-------+----------------------------------+
| Time      | date  | 12    | Start time of pcal average       |
+-----------+-------+-------+----------------------------------+
| Duration  | r*4   | 4     | Duration of pcal average (sec)   |
+-----------+-------+-------+----------------------------------+
| Pcal x 16 |       |       |                                  |
+-----------+-------+-------+----------------------------------+
| Chan_id   | ascii | 32    | Frequency channel identifier     |
+-----------+-------+-------+----------------------------------+
| Freq      | r*4   | 4     | frequency (Hz rel. to chan freq) |
+-----------+-------+-------+----------------------------------+
| Real      | r*4   | 4     | Phasecal vector                  |
+-----------+-------+-------+----------------------------------+
| Imaginary | r*4   | 4     | Phasecal vector                  |
+-----------+-------+-------+----------------------------------+

Record length is fixed.


**type_309**  

The ``type_309`` struct is used for the storage of multi-tone phase calibration data. See :hops:`type_309` for more information.

+----------------+-----------------------+----------------+-------------------------------------------+
| Field Name     | Data Type             | Size (bytes)   | Description                               |
+================+=======================+================+===========================================+
| record_id      | char[3]               | 3              | Standard 3-digit id                       |
+----------------+-----------------------+----------------+-------------------------------------------+
| version_no     | char[2]               | 2              | Standard 2-digit version #                |
+----------------+-----------------------+----------------+-------------------------------------------+
| unused1        | char[3]               | 3              | Unused                                    |
+----------------+-----------------------+----------------+-------------------------------------------+
| su             | int                   | 4              | SU                                        |
+----------------+-----------------------+----------------+-------------------------------------------+
| ntones         | int                   | 4              | Number of tones [0..64]                   |
+----------------+-----------------------+----------------+-------------------------------------------+
| rot            | double                | 8              | ROT at start of AP                        |
+----------------+-----------------------+----------------+-------------------------------------------+
| acc_period     | double                | 8              | Accumulation period (in seconds)          |
+----------------+-----------------------+----------------+-------------------------------------------+
| chan[64]       | struct ch1_tag[64]    | variable       | Array of tone/channel data                |
+----------------+-----------------------+----------------+-------------------------------------------+

Details of struct `ch1_tag` (used in chan[64]):

+----------------+---------------------+----------------+-------------------------------------------+
| Field Name     | Data Type           | Size (bytes)   | Description                               |
+================+=====================+================+===========================================+
| chan_name      | char[8]             | 8              | Channel name                              |
+----------------+---------------------+----------------+-------------------------------------------+
| freq           | double              | 8              | Tone frequency in Hz                      |
+----------------+---------------------+----------------+-------------------------------------------+
| acc[64][2]     | U32                 | 512            | Accumulators: 64 freqs × 2 quads (C, S)   |
+----------------+---------------------+----------------+-------------------------------------------+

