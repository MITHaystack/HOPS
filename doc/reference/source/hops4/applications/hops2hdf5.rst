hops2hdf5
=========

Convert HOPS-format data into an HDF5 file.

Usage
-----

.. code-block:: console

   hops2hdf5 [OPTIONS] input [output]

Description
-----------

`hops2hdf5` converts a HOPS-format file into an HDF5 file. 
If no output file is specified, the result will be stored as `<input-file>.hdf5`. 
This program supports filtering by UUID or shortname to isolate a single object for export.

Positional Arguments
--------------------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Argument
     - Description
   * - ``input``
     - REQUIRED. Name of the input (hops) file to be converted.
   * - ``output``
     - Optional. Name of the output file. Defaults to `<input-file>.hdf5`.

Options
-------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Option
     - Description
   * - ``-h, --help``
     - Print this help message and exit.
   * - ``-m, --message-level INT``
     - Message level to be used, range: -2 (debug) to 5 (silent).
   * - ``-u, --uuid TEXT`` (Excludes: ``--shortname``)
     - Specify and extract a single object by UUID.
   * - ``-s, --shortname TEXT`` (Excludes: ``--uuid``)
     - Specify and extract a single object by shortname (returns first matching object).
   * - ``-i, --input-file TEXT`` (REQUIRED)
     - Name of the input (hops) file to be converted.
   * - ``-o, --output-file TEXT``
     - Name of the output file. If not given, result will be stored in `<input-file>.hdf5`.
