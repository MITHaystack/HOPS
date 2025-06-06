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

In order to facilitate consistency checks of data files, and to ease
programmatic manipulations for filesets, it has been decided to place a special
file identification record at the beginning of each file.  This record is typed
in the same way as all other Mk4 records.  The record type is 000, and the
record length is fixed at 64 bytes.  For various reasons, it is desirable for
this record to be entirely ascii, not least because then it is trivial to
generate a greppable and user-comprehensible summary of many data files.  The
format of the type 000 record is as follows.  All fields are ascii.

+----------+--------+-------------------------------------------------------------------------+
| Field    | Length | Description                                                             |
+==========+========+=========================================================================+
| Type     | 3      | Fixed at 000                                                            |
+----------+--------+-------------------------------------------------------------------------+
| Version  | 2      | Normal meaning                                                          |
+----------+--------+-------------------------------------------------------------------------+
| Unused   | 3      | Spaces                                                                  |
+----------+--------+-------------------------------------------------------------------------+
| Date     | 16     | " yyyyddd-hhmmss ", original file creation date.                        |
|          |        | Note preceding/trailing space                                           |
+----------+--------+-------------------------------------------------------------------------+
| Name     | 32     | Filename relative to data directory, preceded by a space, NULL-padded.  |
|          |        | With current unix correlator filenaming conventions, the maximum        |
|          |        | length of a name starting with the experiment directory is 31           |
|          |        | characters.                                                             |
+----------+--------+-------------------------------------------------------------------------+
| Unused   | 8      | NULLs.                                                                  |
+----------+--------+-------------------------------------------------------------------------+



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

Type 100 (general data description) record format:

+----------------+--------+-------+----------------------------------------------------+
| Field          | Type   | Bytes | Description                                        |
+================+========+=======+====================================================+
| Type           | ascii  | 3     | 100                                                |
+----------------+--------+-------+----------------------------------------------------+
| Version        | ascii  | 2     | 0–99                                               |
+----------------+--------+-------+----------------------------------------------------+
| Unused         | ascii  | 3     | Spaces                                             |
+----------------+--------+-------+----------------------------------------------------+
| procdate       | date   | 12    | Time of correlation                                |
+----------------+--------+-------+----------------------------------------------------+
| Baseline       | ascii  | 2     | Standard baseline ID                               |
+----------------+--------+-------+----------------------------------------------------+
| rootname       | ascii  | 34    | Name of root, from experiment directory            |
+----------------+--------+-------+----------------------------------------------------+
| Qcode          | ascii  | 2     | Correlation “quality”, criteria TBD                |
+----------------+--------+-------+----------------------------------------------------+
| Unused2        | ascii  | 6     | Spaces for padding                                 |
+----------------+--------+-------+----------------------------------------------------+
| Percent done   | r*4    | 4     | 0–100% of scheduled data processed                 |
+----------------+--------+-------+----------------------------------------------------+
| start          | date   | 12    | Time of 1st AP                                     |
+----------------+--------+-------+----------------------------------------------------+
| stop           | date   | 12    | Time of last AP                                    |
+----------------+--------+-------+----------------------------------------------------+
| ndrec          | i*4    | 4     | Total number of data records in file               |
+----------------+--------+-------+----------------------------------------------------+
| nindex         | i*4    | 4     | Number of index numbers present                    |
+----------------+--------+-------+----------------------------------------------------+
| nlags          | i*2    | 2     | Number of lags per type-120 record                 |
+----------------+--------+-------+----------------------------------------------------+
| nblocks        | i*2    | 2     | Number of blocks per index number                  |
+----------------+--------+-------+----------------------------------------------------+

Record length is fixed at 104 bytes.  This record can be thought of as a
"consistency control" record, which should be examined to make sure that
the data are what you think they should be.  The IO library should do
most of this checking, and raise the alarm to the application programmer
when discrepancies are found.  Global hardware configuration information
is also stored here.

Type 101 (index number parameter) record format:

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


Type 120 (sorted lag data) record format:

+------------+-------+----------+----------------------------------------+
| Field      | type  | bytes    | Description                            |
+============+=======+==========+========================================+
| Type       | ascii | 3        | 120                                    |
+------------+-------+----------+----------------------------------------+
| Version    | ascii | 2        | 0-99                                   |
+------------+-------+----------+----------------------------------------+
| corrtype   | ascii | 1        | Binary 1-4, lagdata struct type        |
+------------+-------+----------+----------------------------------------+
| nlags      | i*2   | 2        | Number of lags present                 |
+------------+-------+----------+----------------------------------------+
| Baseline   | ascii | 2        | Standard baseline id                   |
+------------+-------+----------+----------------------------------------+
| rootcode   | ascii | 6        | Standard root suffix                   |
+------------+-------+----------+----------------------------------------+
| Index      | i*4   | 4        | Index into type 101 record             |
+------------+-------+----------+----------------------------------------+
| AP         | i*4   | 4        | Accumulation period number             |
+------------+-------+----------+----------------------------------------+
| Flag       | i*4   | 4        | Up to 32 correlation flags             |
+------------+-------+----------+----------------------------------------+
| Status     | i*4   | 4        | Up to 32 status bits                   |
+------------+-------+----------+----------------------------------------+
| fr_delay   | i*4   | 4        | Mid-AP fractional delay (bits * 2^32)  |
+------------+-------+----------+----------------------------------------+
| delay_rate | i*4   | 4        | Mid-AP delay rate (bits/sysclk * 2^32) |
+------------+-------+----------+----------------------------------------+
| lagdata    | array | variable | Correlation counts                     |
+------------+-------+----------+----------------------------------------+


where lagdata can have any one of four structures:
1. COUNTS_PER_LAG format, lagdata is an array of size nlags, each element
having the structure below.

+---------+-----+---+--------------------------+
| coscor  | i*4 | 4 | Cosine correlation count |
+=========+=====+===+==========================+
| cosbits | i*4 | 4 | Cosine total bit count   |
+---------+-----+---+--------------------------+
| sincor  | i*4 | 4 | Sine correlation count   |
+---------+-----+---+--------------------------+
| sinbits | i*4 | 4 | Sine total bit count     |
+---------+-----+---+--------------------------+

2. COUNTS_GLOBAL format, lagdata is a structure containing an array as shown
below.
cosbits         i*4             4       Cosine total bit count
sinbits         i*4             4       Sine total bit count
array[nlags], each element is:
        coscor  i*4             4       Cosine correlation count
        sincor  i*4             4       Sine correlation count

3. AUTO_PER_LAG format, lagdata is an array of size nlags. each element
having the structure below.
coscor          i*4             4       Cosine correlation count
cosbits         i*4             4       Cosine total bit count

4. AUTO_GLOBAL format,lagdata is a structure containing an array as shown 
below.
cosbits         i*4             4       Cosine total bit count
Unused          ascii           4       Padding for alignment
coscor[nlags]   i*4         nlags*4     Array of correlation counts    

Record length is variable, depending on the number of lags and the mode.
Only one mode, and therefore one lagdata format, is present in any given file.


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


Type 200 (general information) record format:

+--------------+----------+-------+-----------------------------------+
| Field        | type     | bytes | Description                       |
+==============+==========+=======+===================================+
| Type         | ascii    | 3     | 200                               |
+--------------+----------+-------+-----------------------------------+
| Version      | ascii    | 2     | 0-99                              |
+--------------+----------+-------+-----------------------------------+
| Unused       | ascii    | 3     | Spaces                            |
+--------------+----------+-------+-----------------------------------+
| Revision     | i*2 x 10 | 20    | Revision levels for many progs    |
+--------------+----------+-------+-----------------------------------+
| Experiment   | i*4      | 4     | Experiment number                 |
+--------------+----------+-------+-----------------------------------+
| Program      | ascii    | 32    | Observing program name            |
+--------------+----------+-------+-----------------------------------+
| Scan name    | ascii    | 32    | Name of scan                      |
+--------------+----------+-------+-----------------------------------+
| Scantime     | date     | 12    | Scan time to 1 second             |
+--------------+----------+-------+-----------------------------------+
| Start offset | i*4      | 4     | Baseline start (sec) rel. to scan |
+--------------+----------+-------+-----------------------------------+
| Stop offset  | i*4      | 4     | Baseline stop (sec) rel. to scan  |
+--------------+----------+-------+-----------------------------------+
| Corr_date    | date     | 12    | Date of correlation               |
+--------------+----------+-------+-----------------------------------+
| FPT          | date     | 12    | Fourfit processing date           |
+--------------+----------+-------+-----------------------------------+
| FRT          | date     | 12    | Fourfit reference time            |
+--------------+----------+-------+-----------------------------------+


Record length is fixed at 152 bytes.

Type 201 (source information) record format:

+---------------------+-----------+--------------+-------------------------------+
| Field               | type      | bytes        | Description                   |
+=====================+===========+==============+===============================+
| Type                | ascii     | 3            | 201                           |
+---------------------+-----------+--------------+-------------------------------+
| Version             | ascii     | 2            | 0-99                          |
+---------------------+-----------+--------------+-------------------------------+
| Unused              | ascii     | 3            | Spaces                        |
+---------------------+-----------+--------------+-------------------------------+
| Source              | ascii     | 32           | Source name                   |
+---------------------+-----------+--------------+-------------------------------+
| Coord               | sky_coord | 16           | Source coordinates of epoch   |
+---------------------+-----------+--------------+-------------------------------+
| Epoch of ra/dec i*2 | 2         | 1950 or 2000 |                               |
+---------------------+-----------+--------------+-------------------------------+
| Unused2             | ascii     | 2            | Padding                       |
+---------------------+-----------+--------------+-------------------------------+
| Coord_date          | date      | 12           | Ref date for proper motion    |
+---------------------+-----------+--------------+-------------------------------+
| R.A. rate           | r*8       | 8            | Proper motion (rad/sec)       |
+---------------------+-----------+--------------+-------------------------------+
| Dec. rate           | r*8       | 8            | Proper motion (rad/sec)       |
+---------------------+-----------+--------------+-------------------------------+
| Pulsar phase        | r*8 x 4   | 32           | Polynomial of pulse timing    |
+---------------------+-----------+--------------+-------------------------------+
| Pulsar epoch        | r*8       | 8            | reference time for polynomial |
+---------------------+-----------+--------------+-------------------------------+
| Dispersion          | r*8       | 8            | Pulsar dispersion measure     |
+---------------------+-----------+--------------+-------------------------------+
|                     |           |              |                               |
+---------------------+-----------+--------------+-------------------------------+

Record length is fixed at 128 bytes.  This record contains source-specific
information.

Type 202 (baseline information) record format:

+---------------+-------+-------+-----------------------------------+
| Field         | type  | bytes | Description                       |
+===============+=======+=======+===================================+
| Type          | ascii | 3     | 202                               |
+---------------+-------+-------+-----------------------------------+
| Version       | ascii | 2     | 0-99                              |
+---------------+-------+-------+-----------------------------------+
| Unused        | ascii | 3     | Spaces                            |
+---------------+-------+-------+-----------------------------------+
| Baseline      | ascii | 2     | Standard baseline ID              |
+---------------+-------+-------+-----------------------------------+
| Ref intl_id   | ascii | 2     | International 2-char ID           |
+---------------+-------+-------+-----------------------------------+
| Rem intl_id   | ascii | 2     | International 2-char ID           |
+---------------+-------+-------+-----------------------------------+
| Ref name      | ascii | 8     | Station names                     |
+---------------+-------+-------+-----------------------------------+
| Rem name      | ascii | 8     |                                   |
+---------------+-------+-------+-----------------------------------+
| Ref tape      | ascii | 8     | Tape VSNs                         |
+---------------+-------+-------+-----------------------------------+
| Rem tape      | ascii | 8     |                                   |
+---------------+-------+-------+-----------------------------------+
| Nlags         | i*2   | 2     | Number of correlated lags         |
+---------------+-------+-------+-----------------------------------+
| Ref xpos      | r*8   | 8     | Station X coords (meters)         |
+---------------+-------+-------+-----------------------------------+
| Rem xpos      | r*8   | 8     |                                   |
+---------------+-------+-------+-----------------------------------+
| Ref ypos      | r*8   | 8     | Station Y coords (meters)         |
+---------------+-------+-------+-----------------------------------+
| Rem ypos      | r*8   | 8     |                                   |
+---------------+-------+-------+-----------------------------------+
| Ref zpos      | r*8   | 8     | Station Z coords (meters)         |
+---------------+-------+-------+-----------------------------------+
| Rem zpos      | r*8   | 8     |                                   |
+---------------+-------+-------+-----------------------------------+
| U             | r*8   | 8     | fringes/arcsec E-W at 1GHz at FRT |
+---------------+-------+-------+-----------------------------------+
| V             | r*8   | 8     | fringes/arcsec N-S at 1GHz at FRT |
+---------------+-------+-------+-----------------------------------+
| UF            | r*8   | 8     | mHz/arcsec/GHz in R.A. at FRT     |
+---------------+-------+-------+-----------------------------------+
| VF            | r*8   | 8     | mHz/arcsec/GHz in dec. at FRT     |
+---------------+-------+-------+-----------------------------------+
| Ref clock     | r*4   | 4     | Clock offsets (usec) at FRT       |
+---------------+-------+-------+-----------------------------------+
| Rem clock     | r*4   | 4     |                                   |
+---------------+-------+-------+-----------------------------------+
| Ref clockrate | r*4   | 4     | Clock rates (sec/sec)             |
+---------------+-------+-------+-----------------------------------+
| Rem clockrate | r*4   | 4     |                                   |
+---------------+-------+-------+-----------------------------------+
| Ref idelay    | r*4   | 4     | Instrumental delays (usec)        |
+---------------+-------+-------+-----------------------------------+
| Rem idelay    | r*4   | 4     |                                   |
+---------------+-------+-------+-----------------------------------+
| Ref zatm del. | r*4   | 4     | Zenith atmospheric delay (nsec)   |
+---------------+-------+-------+-----------------------------------+
| Rem zatm del. | r*4   | 4     |                                   |
+---------------+-------+-------+-----------------------------------+
| Ref elev      | r*4   | 4     | Elevation of source at FRT (deg)  |
+---------------+-------+-------+-----------------------------------+
| Rem elev      | r*4   | 4     |                                   |
+---------------+-------+-------+-----------------------------------+
| Ref az        | r*4   | 4     | Azimuth of source at FRT (deg)    |
+---------------+-------+-------+-----------------------------------+
| Rem az        | r*4   | 4     |                                   |
+---------------+-------+-------+-----------------------------------+

Record length is fixed at 176 bytes.  This record contains baseline
specific information, independent of fourfit parameters except FRT.

Type 203 (channel information) record format:

+----------------+--------+--------+-----------------------------------------+
| Field          | Type   | Bytes  | Description                             |
+================+========+========+=========================================+
| Type           | ascii  | 3      | 203                                     |
+----------------+--------+--------+-----------------------------------------+
| Version        | ascii  | 2      | 0-99                                    |
+----------------+--------+--------+-----------------------------------------+
| Unused         | ascii  | 3      | Spaces                                  |
+----------------+--------+--------+-----------------------------------------+
| Channels x 32  |        |        |                                         |
+----------------+--------+--------+-----------------------------------------+
|  index         | i*2    | 2      | Index number in type-1 file (0=empty)   |
+----------------+--------+--------+-----------------------------------------+
|  Sample rate   | i*2    | 2      | ksamp/sec                               |
+----------------+--------+--------+-----------------------------------------+
|  refsb         | ascii  | 1      | Ref station sideband (U/L)              |
+----------------+--------+--------+-----------------------------------------+
|  remsb         | ascii  | 1      | Rem station sideband (U/L)              |
+----------------+--------+--------+-----------------------------------------+
|  refpol        | ascii  | 1      | Ref station polarization (R/L)          |
+----------------+--------+--------+-----------------------------------------+
|  rempol        | ascii  | 1      | Rem station polarization                |
+----------------+--------+--------+-----------------------------------------+
|  ref_freq      | r*8    | 8      | Ref station LO freq (Hz)                |
+----------------+--------+--------+-----------------------------------------+
|  rem_freq      | r*8    | 8      | Rem station LO freq (Hz)                |
+----------------+--------+--------+-----------------------------------------+
|  ref_chan_id   | ascii  | 8      | Ref station channel id                  |
+----------------+--------+--------+-----------------------------------------+
|  rem_chan_id   | ascii  | 8      | Rem station channel id                  |
+----------------+--------+--------+-----------------------------------------+

Record length is fixed at 1288 bytes.  This is a copy of the information in all
the (non-mirrored) type 101 records in the type-1 file, regardless of whether
the channels are used in the current fringe-fit.  A "channel" described by a
single entry in the array corresponds to the cross-correlation of a single pair
of channel IDs as described in the $FREQ section of the ovex portion of the root
file.  Note that a fourfit frequency channel may consist of multiple such
"channels", depending on whether this is dual-sideband data, or possibly even
dual polarization data in which RR and LL correlations have been combined before
the fringe fit.

Type 204 (execution setup) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       204
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
ff_version      i*2 x 2         4       Fourfit revision level
Platform        ascii           8       hppa, linux, alpha etc. version
Control file    ascii           96      Control file full pathname
ffcf_date       date            12      Control file modification date
Override        ascii           128     Command line override parms

Record length is fixed at 256 bytes.  The strings are null-terminated.  If
they overflow, the strings are set to null.  This record is just a tracer
of the execution parameters for possible subsequent human intervention.

Type 205 (fourfit setup) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       205
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Utc_central     date            12      Central time of baseline/scan
Offset          r*4             4       FRT offset from central time (sec)
ffmode          ascii           8       Fourfit execution modes
Search parm     r*4 x 6         24      SBD, MBD, Rate windows (usec, usec/sec)
Filters         r*4 x 8(?)      32      Filter thresholds
Start           date            12      Start of requested data span
Stop            date            12      End of requested data span
Ref freq        r*8             8       Fourfit reference frequency (Hz)
ffit_chan x 16
    ffit_chan_id ascii          1       Fourfit channel letter id
    unused2     ascii           1       Padding
    channel     i*2 x 4         8       Index into type 203 array, -1=absent

Record length is fixed at 280 bytes.  This record describes the setup
of the fourfit execution, independent of the AP data.

Type 206 (data filtering) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       206
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Start           date            12      Time at start of 0th AP
first_ap        i*2             2       Number of 1st valid AP
last_ap         i*2             2       Number of last valid AP
Accepted        i*2 x 2 x 16    64      APs accepted by channel/sband
Intg time       r*4             4       Effective integration time (secs)
Accept ratio    r*4             4       % ratio of min/max data accepted
discard         r*4             4       % of data discarded
????????        i*2 x 2 x 16    64      Discards due to reason 1 (?)
????????        i*2 x 2 x 16    64      Discards due to reason 2 (?)
????????        i*2 x 2 x 16    64      Discards due to reason 3 (?)
????????        i*2 x 2 x 16    64      Discards due to reason 4 (?)
????????        i*2 x 2 x 16    64      Discards due to reason 5 (?)
????????        i*2 x 2 x 16    64      Discards due to reason 6 (?)
????????        i*2 x 2 x 16    64      Discards due to reason 7 (?)
????????        i*2 x 2 x 16    64      Discards due to reason 8 (?)
ratesize        i*2             2       Size of fringe rate transform
MBD size        i*2             2       Size of MBD transform
SBD size        i*2             2       Size of SBD transform
Unused2         ascii           6       Padding

Record length is fixed at 556 bytes.  This record describes the data
filtering action taken by fourfit in detail.  The AP flagging criteria
for Mk4 will undoubtedly be different, but this is a start.  Info is
much more detailed than Mk3, since records are kept by channel/sband.
The record also describes the array sizes that fourfit decided to use.

Type 207 (phasecal and error rate) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       207
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Pcal_mode       i*4             4       10 * stn1 + stn2, values 1-3 in control.h
Pcal ampl.      r*4 x 4 x 16    256     Pcal amp by chan/sband, ref/remote
Pcal phase      r*4 x 4 x 16    256     Pcal phases
Pcal offset     r*4 x 4 x 16    256     Pcal offsets (manual pcals)
Pcal freq       r*4 x 4 x 16    256     Pcal frequencies
pcalrate ref    r*4             4       Pcal rate reference station
pcalrate rem    r*4             4       Pcal rate remote station
errorate        r*4 x 2 x 16    128     Error rates by chan, ref/remote

Record length is fixed at 1172 bytes.  The phasecal and errorate
numbers are as used by fourfit after extraction from the type-3 files,
and interpretation in terms of the fourfit input parameters.  Errorates
are by channel, translated from track error rates by arithmetic average
of all contributing tracks.

Type 208 (solution parameter)record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       208
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Quality         ascii           1       Fringe quality 0 to 9
Errorcode       ascii           1       Currently supported B,D,E,G,H
Tape qcode      ascii           6       Backwards compatibility? NYI
Adelay          r*8             8       Total apriori delay at FRT (usec)
Arate           r*8             8       Total apriori rate at FRT (usec/sec)
Aaccel          r*8             8       Total apriori acceleration (usec/sec/sec)
Obs. delay      r*8             8       Total e.c. group delay (usec)
Obs. sbdelay    r*8             8       Total e.c. sband delay (usec)
Obs. rate       r*8             8       Total e.c. rate (usec/sec)
Ref. delay      r*8             8       Total mk3-frame group delay (usec)
Ref. sbdelay    r*8             8       Total mk3-frame sband delay (usec)
Ref. rate       r*8             8       Total mk3-frame rate (usec/sec)
Resid. delay    r*4             4       MBD residual to model (usec)
Resid sbd       r*4             4       SBD residual to model (usec)
Resid rate      r*4             4       Rate resid to model (usec/sec)
MBD error       r*4             4       MBD error calc'd from data (usec)
SBD error       r*4             4       SBD error calc'd from data (usec)
Rate error      r*4             4       Rate error calc'd from data (usec/sec)
ambig.          r*4             4       Group delay ambiguity (usec)
Coh ampl.       r*4             4       Coherent amplitude (corr. coeff.)
Inc ampl. 1     r*4             4       Amp. from incoherent segment addition
Inc ampl. 2     r*4             4       Amp. from incoherent channel addition
SNR             r*4             4       SNR in sigmas
Prob. false     r*4             4       Probability of false detection
totphase        r*4             4       Total e.c. fringe phase (deg)
totphase_ref    r*4             4       Total mk3-frame fringe phase (deg)
resphase        r*4             4       Residual earth-centered phase (deg)

Record length is fixed at 120 bytes.  This record contains the essentials of
the fringe fit solution.

WARNING:
--------
Type 2xx record formats below this point are considerably less stable, and are
likely to change extensively in response to development of fourfit.

Type 210 (channel data) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       210
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Amp-phase       r*4 x 2 x 16    128     Amp/phase by channel resid to model

Record length is fixed at 134 bytes.  The entries are equivalent to the entries
in the Mk3 type-4500 record.

Type 211 (data slices) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       211
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Rate spectrum   r*4 x r         4r      Rate spectrum at peak
MBD function    r*4 x m         4m      MBD resolution function at peak
SBD function    r*4 x s         4s      SBD function at peak
Xpower total    r*4 x s         4s      Crosspower spectrum avg'd across chans
Xpower chans    r*4 x s x c     4sc     Xpower spectra for each freq channel
MBD solutions   r*4 x s         4s      Best MBD values for each SBD chan.
Rate spectrum   r*4 x r x s     4sr     Rate spectra at best MBD for each SBD chan.

The numbers r, m, s and c are the sizes of the rate spectrum, multiband delay
function, singleband delay function, and channel array, respectively.  The numbers
r, m and s can be found in the type 205 record, and the number c can be found
in the type 204 record.  Record length is variable, given by 
4*(r + m + s*(3+c+r)) + 6 bytes.  This record gives the information required to 
reconstruct everything that appears in the upper graphical portion of the Mk3 
fringe plot, plus some additional information.  It can be used to generate 
high-resolution graphics by a suitable program.

Type 212 (AP data) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       212
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
channel         i*2             2       Channel number for this record
SBD chan        i*2             2       SBD chan. no. for this record
data            r*4 x 3 x a     12a     Amp, phase, pcal phase by a.p.

The number a is the number of APs in the processed data span.  Record length 
is variable, given by 10 + 12*a.  There are multiple type-212 records.  
The data are rotated to the fourfit solution, unless additional SBD channels 
are dumped in which case those records are rotated to the center of the SBD 
channel in question.

Type 220 (fringe plot) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       220
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Fringe plot     ascii           15360   Standard fringe plot image

Record length is fixed at 15366 bytes.  This is simply a record-typed version
of the Mk3 fringe plot image.

Type 221 (postscript plot) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       221
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Post. plot      ascii           ??      High-res postscript graphics of fringe


Type 3 (Station unit data) file record formats
-----------------------------------------------------------

The type 3 files contain station-specific information for each of the
stations involved in the scan.  They are initially created by genaroot, which
writes the type 300 and 301/2 records.  These contain the delay and phase spline
polynomials which describe the correlator model, and which drive the station
unit in its delay compensation and frame header construction.  The files are
appended to by the station unit manager software during or after the
correlation, with information on track error rates, state counts, and extracted
phasecal values in type 304, 306 and 308 records respectively.

Type 300 (station ID and model parameter) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       300
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Id              ascii           1       1-char vex letter code
Intl_id         ascii           2       2-char international station code
Name            ascii           32      Full station name
Unused          ascii           1       Padding for alignment
Model_date      date            12      Start time for 1st spline
Model interval  r*4             4       Spline interval in seconds
Nsplines        i*2             2       Number of splines in scan

Record length is fixed at 64 bytes.

Type 301 (delay polynomial coefficient) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       301
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Interval        i*2             2       Sequential model interval number
Chan_id         ascii           32      Frequency channel identifier
Unused          ascii           6       Padding for alignment
Delay_spline    r*8 x 6         48      Delay spline coefficients

Record length is fixed at 58 bytes.

Type 302 (phase polynomial coefficient) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       302
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Interval        i*2             2       Sequential model interval number
Chan_id         ascii           32      Frequency channel identifier
Unused          ascii           6       Padding for alignment
Phase_spline    r*8 x 6         48      Phase spline coefficients

Record length is fixed at 96 bytes.

Type 303 ("raw" track error statistics) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       303
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Format TBD (PAH/SRS)

Type 304 ("cooked" track error statistics) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       304
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Time            date            12      Start time of current error stats
Duration        r*4             4       Duration of current error stats (sec)
Statistics x 64
   Error_rate   r*4             4       Fraction
   Frames       i*4             4       Count
   Bad_frames   i*4             4       Count
   Slip_sync    i*4             4       Count
   Missing_sync i*4             4       Count
   CRC_error    i*4             4       Count

Record length is fixed at 1560 bytes.

Type 305 ("raw" state count) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       305
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Format TBD (PAH/SRS)

Type 306 ("cooked" state count) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       306
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Time            date            12      Start time of current counts
Duration        r*4             4       Duration of current counts (sec)
Stcount x 16
    Chan_id     ascii           32      Frequency channel identifier
    Bigpos      i*4             4       Count of big positive voltage samples
    Pos         i*4             4       Count of small positive voltage samples
    Neg         i*4             4       Count of small negative voltage samples
    Bigneg      i*4             4       Count of big negative voltage samples

Record length is fixed at 792 bytes.

Type 307 ("raw" phase cal value) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       307
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Format TBD (ARW).


Type 308 ("cooked" phase cal value) record format:

Field           type            bytes   Description
-----           ----            -----   -----------
Type            ascii           3       308
Version         ascii           2       0-99 
Unused          ascii           3       Spaces
Time            date            12      Start time of pcal average
Duration        r*4             4       Duration of pcal average (sec)
Pcal x 16
    Chan_id     ascii           32      Frequency channel identifier
    Freq        r*4             4       frequency (Hz rel. to chan freq)
    Real        r*4             4       Phasecal vector
    Imaginary   r*4             4       Phasecal vector

Record length is fixed at 728 bytes.



















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






