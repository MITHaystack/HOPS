hops2flat
=========

Convert HOPS-format data into a flat directory layout with JSON or CBOR metadata.

Usage
-----

.. code-block:: console

   hops2flat [OPTIONS] input [output]

Description
-----------

`hops2flat` converts a HOPS-format file into a flat directory structure.  Table container 
data is split into meta-data and data. The meta-data is stored in a separate JSON file, while the dense numerical data 
is stored as a flat binary file named according to the object UUID and numpy d-type. The meta data export supports 
JSON (ASCII) or CBOR. Filtering by UUID can be applied to limit output to a single object.
If no output directory is specified, results are stored in a new directory called `<input-file>.flat`.

Positional Arguments
--------------------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Argument
     - Description
   * - ``input``
     - REQUIRED. Path to the input HOPS-format file.
   * - ``output``
     - Optional. Output directory name. Defaults to `<input>.flat`.

Options
-------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Option
     - Description
   * - ``-h, --help``
     - Print this help message and exit.
   * - ``-d, --detail INT``
     - Set level of detail in output (0 = low, 3 = high). Default: 3.
   * - ``-m, --message-level INT``
     - Set verbosity level. Range: -2 (debug) to 5 (silent).
   * - ``-p, --pretty-print UINT`` (Excludes: ``--cbor``)
     - Indent JSON metadata using the specified number of spaces. Default: disabled.
   * - ``-u, --uuid TEXT`` (Excludes: ``--shortname``)
     - Extract and export a single object by UUID.
   * - ``-s, --shortname TEXT`` (Excludes: ``--uuid``)
     - Extract and export a single object by shortname (returns first match).
   * - ``-t, --trim-uuid INT``
     - Truncate UUIDs in filenames to the specified number of characters.
   * - ``-c, --cbor`` (Excludes: ``--pretty-print``)
     - Output metadata in CBOR binary format instead of JSON.
   * - ``-i, --input-file TEXT`` (REQUIRED)
     - Same as positional ``input``; path to the input HOPS file.
   * - ``-o, --output-dir TEXT``
     - Same as positional ``output``; name of output directory.
