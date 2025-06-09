hops2json
=========

Convert HOPS-format data into JSON.

Usage
-----

.. code-block:: console

   hops2json [OPTIONS] input [output]

Description
-----------

`hops2json` reads a HOPS-format file and converts it to JSON. You can control output verbosity, formatting, and filter by specific UUIDs or shortnames. 
If no output file is specified, the result is saved in a new file called `<input-file>.json`.

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
     - Optional. Path to the output JSON file. Defaults to `<input>.json`.

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
     - Level of detail in output. Range: 0 (low) to 4 (high). Default: 4.
   * - ``-m, --message-level INT``
     - Set verbosity level. Range: -2 (debug) to 5 (silent).
   * - ``-p, --pretty-print UINT``
     - Indent JSON output using the specified number of spaces. Default: disabled.
   * - ``-u, --uuid TEXT`` (Excludes: ``--shortname``)
     - Extract and return a single object matching this UUID.
   * - ``-s, --shortname TEXT`` (Excludes: ``--uuid``)
     - Extract and return a single object matching this shortname (returns first match).
   * - ``-i, --input-file TEXT`` (REQUIRED)
     - Same as the positional ``input``; path to the input HOPS file.
   * - ``-o, --output-file TEXT``
     - Same as the positional ``output``; path to the output JSON file.
