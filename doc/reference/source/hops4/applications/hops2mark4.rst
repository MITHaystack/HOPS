hops2mark4
==========

Convert HOPS4 format data to legacy Mark4 format.

Usage
-----

.. code-block:: console

   hops2mark4 [OPTIONS] input_dir [output_dir]

Description
-----------

`hops2mark4` reads HOPS format data files and converts it to the legacy Mark4 correlator output. 
You may optionally specify an output directory, otherwise output will be written to the input directory.

Positional Arguments
--------------------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Argument
     - Description
   * - ``input_dir``
     - REQUIRED. Path to the input directory containing Mark4 data.
   * - ``output_dir``
     - Optional. Directory where output files will be written. If not specified, output is placed in the input directory.

Options
-------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Option
     - Description
   * - ``-h, --help``
     - Print this help message and exit.
   * - ``-i, --input-dir TEXT`` (REQUIRED)
     - Same as positional ``input_dir``; input directory path.
   * - ``-o, --output-dir TEXT``
     - Same as positional ``output_dir``; output directory path.
   * - ``-m, --message-level INT``
     - Set verbosity level. Range: -2 (debug) to 5 (silent).
