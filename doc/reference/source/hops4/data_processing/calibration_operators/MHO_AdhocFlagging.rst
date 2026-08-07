MHO\_AdhocFlagging
==================

Purpose
-------
``MHO_AdhocFlagging`` reads up to two ASCII flag files (one for the reference
station, one for the remote station) and zeroes out data weights for (channel,
accumulation period) combinations where the flag data indicates the data should
be discarded. This operator is a direct port of the legacy
fourfit ``adhoc_flag()`` capability into the HOPS4 calibration framework.

Control File Trigger
--------------------
- **Keyword:** ``adhoc_flag_file``
- **Category:** flagging
- **Priority:** 3.5

.. list-table:: Parameters for ``adhoc_flag_file``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - flag_file
     - string
     - Path to the ASCII flag file for the station.

The control file statement may be scoped to one or both stations via station
identifiers. Both the reference and remote station can have independent flag
files. If a station has no flag file, its contribution is treated as
all ``0xFF`` (retain everything).

Input Data
----------
This operator acts on the ``weight_type`` container
(data weights only; visibilities are untouched).

Algorithm
---------
The operator performs the following steps:

**Initialization (``InitializeInPlace``):**

1. If both flag files are empty, the operator becomes a no-op.
2. Extract the scan start time from the weight container's ``start`` metadata
   tag, and convert it to fractional days since the beginning of the year (fpday).
3. Derive the accumulation period (AP) duration from the time axis by
   subtracting consecutive time values.
4. Load and parse both flag files into in-memory lookup tables.
   Each data line in a flag file consists of a fractional day time and a
   hexadecimal string encoding up to 64 flag bytes (one per frequency channel).
5. Each hex byte is decoded per the ``DecodeHexToken`` routine: the string
   is consumed nibble by nibble (upper then lower) to fill 64 bytes. When the hex
   string is shorter than 128 hex characters, the last nibble pair is repeated
   for all remaining bytes.

**Execution (``ExecuteInPlace``):**

1. Iterate over every frequency channel in the weight container.
2. For each channel, retrieve the ``net_sideband`` label (``U`` for upper
   sideband, ``L`` for lower sideband). Channels without this label are skipped.
3. Map the channel ordinal to a flag-byte index (``fr = ch % 64``).
4. For each accumulation period (AP):

   a. Compute the AP center time in fractional days since the beginning of the year:

      .. math::

         t_{\rm fpday} = t_{\rm scan\_start\_fpday} + \frac{t_{\rm AP} + 0.5 \cdot t_{\rm acc}}{86400}

   b. Perform a lower-bound lookup in both the reference and remote flag tables to find the last row whose time is <= t_fpday. If the time falls outside the table's range, a sentinel all (``0xFF``) row is returned (no flagging).
   c. Combine the two station bytes: ``combined = ref_byte[fr] & rem_byte[fr]``.
   d. Determine retention based on sideband:

      - USB: retain when ``(combined & 0x55) != 0``
      - LSB: retain when ``(combined & 0xAA) != 0``

      Note: the bit masks follow the legacy ``adhoc_flag.c`` convention, which is inverted relative to the vhelp documented bit assignments (USB uses ``0x55``, LSB uses ``0xAA``).

   e. If the data is not retained, zero out all polarization-product entries for that (channel, AP).

5. After all channels and APs are processed, recompute the total summed weights and the effective AP count (number of APs with non-zero summed weights), and update both the container metadata and the parameter store.

Effect on Data
--------------
For (channel, AP) combinations where the combined flag bytes indicate the data should be discarded, all data weights for every polarization product at that (channel, AP) are multiplied by zero. The operator then recomputes the total summed weights and the effective number of unflagged accumulation periods, storing the results in the container metadata and the parameter store. Visibilities are not modified.
