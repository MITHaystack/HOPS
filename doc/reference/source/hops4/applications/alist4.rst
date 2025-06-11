alist4
======

Usage
-----

    ``alist4 [OPTIONS] input_files...``

Description
-----------

    Tool to process a list of HOPS4 fringe (.frng) files and produce a summary file in A-format
    The summary file can be read by ``aedit`` or other tools that understand the format.

Positional Arguments
--------------------

.. list-table::
   :header-rows: 1

   * - Argument
     - Description
   * - ``input_files`` (TEXT ..., REQUIRED)
     - List of the .frng files to process

Options
-------

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - ``-h``, ``--help``
     - Print this help message and exit
   * - ``-o`` TEXT, ``--output-file`` TEXT
     - Name of the output file (default: ``alist.out``)
   * - ``-m`` INT, ``--message-level`` INT
     - Message verbosity level, range: ``-2`` (debug) to ``5`` (silent)
   * - ``-c`` TEXT, ``--comment-character`` TEXT
     - Character indicating a comment line (default: ``*``)
   * - ``-v`` INT, ``--version`` INT
     - Alist format version (default: ``6``)
   * - ``-j``, ``--json``
     - Generate a JSON summary file instead of an alist-formatted file
   * - ``-i`` TEXT ..., ``--input-files`` TEXT ... (REQUIRED)
     - List of the files to process (alternative form of positional argument)
