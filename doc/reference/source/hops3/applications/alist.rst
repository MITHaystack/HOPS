alist
=====

Synopsis
--------

``alist`` is a utility that summarizes correlator data files into A-format
files. These files can be used as input to other programs like ``aedit``.

**Description**

``alist`` produces a one-line summary for each binary correlator data file, written in A-format.
The summary file can be read by ``aedit`` or other tools that understand the format.

Currently, only **type 2 fourfit fringe files** are supported.

- Supported output format versions: up to **version 5**
- A future **version 6** is planned, offering higher precision.

Usage
-----

.. code-block:: bash

    alist [-ff] [-fr] [-o output file] [-v version] [data file list]

.. note::
   All option flags must appear *before* the data file list.

Options
-------

- ``-o <outfile>``  
  Specify the output file name.  
  **Default**: ``alist.out`` in the current directory.

- ``-v <version>``  
  Force output to use a specific A-format version (integer).  
  **Default**: the highest supported version.

- ``-ff``, ``-fr``  
  Flags exist but are undocumented here -- refer to the source code or extended
  documentation if needed.

Arguments
---------

- **data file list**  
  One or more binary data files to summarize. These can be specified in several formats:

  - Individual filenames
  - Scan directories containing data files
  - Experiment directories

``alist`` will recursively search directories for valid data files. Be cautious with large
input lists; overly long argument lists may overflow the Unix argument buffer. Prefer
specifying experiment directories to avoid this issue.

Use coarse filtering through file selection. For advanced filtering, use the ``aedit`` tool instead.


