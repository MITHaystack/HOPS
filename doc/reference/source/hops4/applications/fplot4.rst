fplot4
======

Plot and analyze fringe data from input directories or files.

Usage
-----

.. code-block:: console

   fplot4 [OPTIONS] input...

Description
-----------

`fplot4` processes and plots fringe files or directories containing fringe results. It supports message filtering, baseline selection, and exporting plots.

Positional Arguments
--------------------

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - Argument
     - Description
   * - input
     - REQUIRED. Name of the input directory, fringe file, or list of fringe files.

Options
-------

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - Option
     - Description
   * - ``-h, --help``
     - Print this help message and exit.
   * - ``-d, --diskfile TEXT``
     - Name of the file in which to save the fringe plot.
   * - ``-b, --baseline TEXT``
     - Baseline or baseline:frequency_group selection (e.g. ``GE`` or ``GE:X``).
   * - ``-M, --message-categories TEXT ...``
     - Limit the allowed message categories to only those specified by the user. Available categories include:

       ::

         main
         calibration
         containers
         control
         fringe
         file
         initialization
         mk4interface
         utilities
         vex
         python_bindings

       If this option is not used, all categories are allowed by default.
   * - ``-m, --message-level INT``
     - Message verbosity level. Range: -2 (debug) to 5 (silent).
   * - ``-n, --no-plot``
     - Do not display the fringe plot.
   * - ``-P, --polprod TEXT``
     - Plot only files matching this polarization product (e.g. ``XX``, ``I``, or ``RR+LL``).
   * - ``-i, --input TEXT ...`` (REQUIRED)
     - Same as the positional argument; specifies the input directory, fringe file, or list of files.
