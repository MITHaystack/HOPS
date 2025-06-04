fourfit4
========

**Usage**
    ``fourfit4 [OPTIONS] input [SUBCOMMAND]``

**Description**
    Command-line tool for fringe-fitting with various configuration options.

**Positional Arguments**

.. list-table::
   :header-rows: 1

   * - Argument
     - Description
   * - ``input`` (TEXT, REQUIRED)
     - Name of the input directory (scan) or root file

**Options**

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - ``-h``, ``--help``
     - Print this help message and exit
   * - ``-a``, ``--accounting``
     - Perform run-time accounting/profiling
   * - ``-b`` TEXT, ``--baseline`` TEXT
     - Baseline or baseline:frequency_group selection (e.g., ``GE`` or ``GE:X``)
   * - ``-c`` TEXT, ``--control-file`` TEXT
     - Specify the control file
   * - ``-d`` TEXT, ``--disk-file`` TEXT
     - Specify the file name where the plot will be saved
   * - ``-e``, ``--exclude-autocorrs``
     - Exclude auto-correlations from fringe-fitting
   * - ``-f`` INT, ``--first-plot-channel`` INT
     - Specifies the first channel displayed in the fringe plot (ignored, not yet implemented)
   * - ``-M`` TEXT ..., ``--message-categories`` TEXT ...
     - Limit the allowed message categories to only those which the user specifies. Available categories:
       
       - ``main``
       - ``calibration``
       - ``containers``
       - ``control``
       - ``fringe``
       - ``file``
       - ``initialization``
       - ``mk4interface``
       - ``utilities``
       - ``vex``
       - ``python_bindings``

       If not used, all categories are allowed.
   * - ``-m`` INT, ``--message-level`` INT
     - Message verbosity level, range: ``-2`` (debug) to ``5`` (silent)
   * - ``-n`` INT, ``--nplot-channels`` INT
     - Number of channels to display in the fringe plot (ignored, not yet implemented)
   * - ``-p``, ``--plot``
     - Generate and show fringe plot on completion
   * - ``-r`` TEXT, ``--refringe-alist`` TEXT
     - Alist file for refringing (ignored, not yet implemented)
   * - ``-s`` INT, ``--ap-per-segment`` INT
     - Specify the APs to be averaged per plot segment
   * - ``-t``, ``--test-mode``
     - If passed, no output is written
   * - ``-u``, ``--update-mode``
     - Ignored, not yet implemented
   * - ``-P`` TEXT, ``--polprod`` TEXT
     - Polarization product (e.g., ``XX``, ``I``, ``RR+LL``)
   * - ``-T`` TEXT, ``--reftime`` TEXT
     - Fourfit reference time (ignored, not yet implemented)
   * - ``-X`` INT, ``--xpower-output`` INT
     - append cross power data with fringe solution applied, specifying the axis along which data should be summed, options are:
       
       - ``-1``: no export (default)
       - ``0``: none
       - ``1``: channel
       - ``2``: time/AP
       - ``3``: sub-channel
   * - ``-i`` TEXT, ``--input`` TEXT (REQUIRED)
     - Name of the input directory (scan) or root file
   * - ``-k``, ``--mark4-output``
     - Write output files in Mark4 type_2xx format

**Subcommands**

.. list-table::
   :header-rows: 1

   * - Subcommand
     - Description
   * - ``set``
     - Pass control file parameters and related syntax on the command line
