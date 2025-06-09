difx2hops
=========

Convert DiFX output to HOPS-readable format.

Usage
-----

.. code-block:: console

   difx2hops [OPTIONS] input_dirs... [output_dir]

Description
-----------

`difx2hops` processes one or more DiFX output directories and generates HOPS-compatible output. 
Various optional filtering, labeling and other configuration options (similar to difx2mark4) are supported.

Positional Arguments
--------------------

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Argument
     - Description
   * - ``input_dirs``
     - REQUIRED. One or more input directories to be converted.
   * - ``output_dir``
     - Optional. Output directory where results (HOPS format) will be written. If unspecified, defaults to ``<cwd>/<exp-num>``.

Options
-------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Option
     - Description
   * - ``-h, --help``
     - Print this help message and exit.
   * - ``-i, --input-dirs TEXT ...`` (REQUIRED)
     - Same as positional ``input_dirs``; directories to convert.
   * - ``-o, --output-dir TEXT``
     - Same as positional ``output_dir``; destination for output files.
   * - ``-m, --message-level INT``
     - Set message verbosity level. Range: -2 (debug) to 5 (silent).
   * - ``-s, --scode TEXT``
     - Path to file mapping 2-character station codes to 1-character Mk4 station IDs. Format per line: ``X Xx``.
   * - ``-e, --exp-num INT``
     - Experiment identification number.
   * - ``-r, --raw-mode``
     - Enable raw mode (skip auto-correlation normalization).
   * - ``-P, --preserve-difx-names``
     - Use original DiFX scan names instead of DOY-HHMM format.
   * - ``-b, --band [TEXT, FLOAT, FLOAT] ...`` (Excludes: ``--legacy-bands``)
     - Define frequency band codes. Each band is a triplet: ``<code> <freq_low> <freq_high>`` in MHz. If unspecified and ``-L`` is not passed, no band assignment is made.
   * - ``-L, --legacy-bands`` (Excludes: ``--band``)
     - Use legacy band definitions (default: off). The bands are:

       ::

         B:  (0,     1e+06) MHz
         I:  (100,   150)   MHz
         G:  (150,   225)   MHz
         P:  (225,   390)   MHz
         L:  (390,   1750)  MHz
         S:  (1750,  3900)  MHz
         C:  (3900,  6200)  MHz
         X:  (6200,  10900) MHz
         K:  (10900, 36000) MHz
         Q:  (36000, 46000) MHz
         V:  (46000, 56000) MHz
         W:  (56000, 100000) MHz

       Narrower bands take precedence during assignment.
   * - ``-C, --legacy-station-codes``
     - Use legacy station code map for assigning Mk4 station IDs.
   * - ``-g, --freq-groups TEXT ...``
     - Include only the specified frequency groups.
   * - ``-w, --bandwidth FLOAT``
     - Include only channels matching this bandwidth (in MHz).
   * - ``-a, --attach-difx-input``
     - Attach the original DiFX ``.input`` file to the visibility object tags.
