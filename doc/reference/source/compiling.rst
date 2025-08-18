=============
Compiling
=============

Build Instructions
------------------

To configure the build system with the default options and compile the code using CMake, execute the following:

.. code-block:: bash

   cd <hops-source>
   mkdir build
   cd build
   cmake ../
   make && make install

For faster builds, you may use parallel compilation with ``-j N`` (e.g., ``make -j 8``).

After building, update your ``PATH`` variable by executing the environment setup script with:

.. code-block:: bash

   source <hops-install>/bin/hops.bash

If you are using the default install directory, you should see something like:

::

   HOPS install directory set to /home/oper/HOPS/x86_64-4.0.0

The default installation directory (on x86 systems) will be: ``<build-dir>/../x86_64-4.00``. 
To override this, you may specify the install prefix as follows:

.. code-block:: bash

   HOPS4_INSTALL_DIR="~/hops-install"
   cmake -DCMAKE_INSTALL_PREFIX=${HOPS4_INSTALL_DIR}

Use the command ``ccmake`` (CMake curses GUI) to configure options manually, and to set paths to dependencies which may not be detected automatically. 
On the first run, press 'c' to configure the build, 'e' to exit and edit, then 'c' to re-configure, and then 'g' to generate.

For example, the options table provided by ``ccmake`` may look like the following:

::

  BASH_PROGRAM                    */usr/bin/bash
  BC_PROGRAM                      */usr/bin/bc
  BUILD_DOXYGEN_REF               *OFF
  BUILD_LATEX_DOCS                *OFF
  CMAKE_BUILD_TYPE                *Release
  CMAKE_INSTALL_PREFIX            */home/oper/HOPS/x86_64-4.0.0
  CPGPLOT_LIBRARY                 */usr/lib/libcpgplot.so
  ENABLE_COLOR_MSG                *ON
  ENABLE_DEBUG_MSG                *ON
  ENABLE_EXTRA_VERBOSE_MSG        *OFF
  ENABLE_SNAPSHOTS                *OFF
  ENABLE_STEPWISE_CHECK           *OFF
  EXTRA_WARNINGS                  *OFF
  GFORTRAN_LIB                    */lib/x86_64-linux-gnu/libgfortran.so.5
  GS_EXE                          */usr/bin/gs
  HOPS3_DISABLE_WARNINGS          *ON
  HOPS3_PYTHON_EXTRAS             *ON
  HOPS3_USE_ADHOC_FLAGGING        *ON
  HOPS_BUILD_EXTRA_CONTAINERS     *OFF
  HOPS_CACHED_TEST_DATADIR        */home/oper/HOPS/x86_64-4.0.0/data/test_data
  HOPS_ENABLE_DEV_TODO            *OFF
  HOPS_ENABLE_TEST                *ON
  HOPS_IS_HOPS4                   *OFF
  HOPS_PYPI_MANAGE_DEPS           *OFF
  HOPS_USE_CUDA                   *OFF
  HOPS_USE_DIFXIO                 *OFF
  HOPS_USE_FFTW3                  *ON
  HOPS_USE_HDF5                   *OFF
  HOPS_USE_MATPLOTPP              *ON
  HOPS_USE_MPI                    *OFF
  HOPS_USE_OPENCL                 *OFF
  HOPS_USE_PYBIND11               *ON
  PYBIND11_FINDPYTHON             *OFF
  PYBIND11_INSTALL                *OFF
  PYBIND11_INTERNALS_VERSION      *
  PYBIND11_NOPYTHON               *OFF
  PYBIND11_SIMPLE_GIL_MANAGEMENT  *OFF
  PYBIND11_TEST                   *OFF
  TAR_PROGRAM                     */usr/bin/tar
  WGET_PROGRAM                    */usr/bin/wget

Dependencies
------------

The minimum required dependencies (to build HOPS4) are:

1. cmake, cmake-curses-gui, GNU make, and bash
2. A C++11 compiler (gcc >= 4.8.5 or clang >= 3.6)
3. Python 3 and pip
4. ``wget`` and ``jq`` (not required, but used for testing)

To install the software on Ubuntu/Debian, first install the dependencies as follows:

.. code-block:: bash

   sudo apt-get install build-essential cmake cmake-curses-gui python3-dev python3-pip wget jq

Similarly, to get the dependencies on on RHEL/Fedora, do:

.. code-block:: bash

   sudo dnf install gcc-c++ cmake cmake-gui python3-devel python3-pip wget jq

In some circumstances certain RHEL-based distributions may require the following additional package repositories to be added:

.. code-block:: bash

   sudo dnf config-manager --set-enabled crb
   dnf install epel-release

While not strictly required by HOPS4, the Fast Fourier Transform library FFTW3 and gnuplot are highly recommended. The FFTW3 library will accelerate 
fringe fitting by utilizing optimized FFT routines, and gnuplot will enable faster plotting (without requring python). These can be installed with:

.. code-block:: bash

   sudo apt-get install libfftw3-dev gnuplot

or

.. code-block:: bash

   sudo dnf install fftw-devel gnuplot

The Python packages required by HOPS4 can be installed via pip.
If the build flag ``HOPS_PYPI_MANAGE_DEPS`` is set to ``ON``, then pip will automatically locate and locally install the necessary python dependencies
in the HOPS install directory when ``make install`` is run. If this flag is set to OFF, then it is expected that users will manage they python dependencies (numpy, matplotlib, scipy, future)
manually themselves. Be aware, that if these packages (numpy, matplotlib, scipy) are already installed system wide, and ``HOPS_PYPI_MANAGE_DEPS`` is also set to ``ON``, then
you may run into a package conflict between the two installations, so it is best to pick one strategy and stick to it.

If you desire to build the original HOPS3 software there are additional requirements, these are:

1. Python 3.x
2. FFTW3
3. PGPLOT
4. X11
5. GNU Fortran
6. Ghostscript

To install these on on Ubuntu/Debian, do:

.. code-block:: bash

   sudo apt-get install python3-dev python3-pip pgplot5 libgfortran5 libfftw3-dev libx11-dev \
                        gnuplot binutils libxpm-dev ghostscript ghostscript-x

Whereas to install these on RHEL/Fedora, do:

.. code-block:: bash

   sudo dnf install python3-devel python3-pip gcc-gfortran fftw-devel libX11-devel gnuplot \
                    binutils libXpm-devel ghostscript

The package PGPLOT must be installed manually on Fedora/RHEL since there is no readily available package. See the document ``<hops-source>/doc/notes/pgplot.txt``
for additional information on how to do this.

Testing the Build
-----------------

In order to run the test suite after installation, you must first download the test data with the ``testdata_download_all.sh`` script:

.. code-block:: bash

   cd <hops-git>
   cd ./build
   source <hops-install>/bin/hops.bash
   testdata_download_all.sh
   make test

Note that some some tests require a CI/CD key in order to download non-public data, if this data is not present, then these tests will be skipped.
Also, a terminal width of at least 95 characters is needed to avoid line wrap in the test output.

Building the Documentation
--------------------------

To build the documentation using Doxygen and Sphinx, set ``BUILD_DOCS=ON`` and run:

.. code-block:: bash

   make reference && make install

The output will be placed in: ``<hops-install>/doc/reference``, with the index file at: ``<hops-install>/doc/reference/index.html``.
You will need doxygen, sphinx, and python packages breathe and myst_parser to build the documentation files.

Getting Help
------------

For questions or comments, please contact the developer mailing list:

``hops-dev@mit.edu``

License and Authorship
-----------------------

See the :doc:`license` and :doc:`authors` sections for information on license and authorship.
