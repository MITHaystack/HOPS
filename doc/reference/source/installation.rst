====================
Getting the software
====================

If you are comfortable using ``git``, the easiest way to get the latest version
of the HOPS4 software is to clone it from the github repository:

.. code-block:: bash

   git clone https://github.com/MITHaystack/HOPS.git

If you would rather not use ``git``, or want a specific tagged release version,
you can navigate to the release page here:

https://github.com/MITHaystack/HOPS/releases

and download the specific package you need.

=============
Compiling
=============

Compiling HOPS4 is supported on GNU/Linux systems with POSIX support.

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

   HOPS install directory set to /home/oper/HOPS/x86_64-4.0.3

The default installation directory (on x86 systems) will be: ``<build-dir>/../x86_64-4.X.Y``. Where "4.X.Y"
corresponds to the current software version string.
To override this, you may specify the install prefix as follows:

.. code-block:: bash

   HOPS4_INSTALL_DIR="~/hops-install"
   cmake -DCMAKE_INSTALL_PREFIX=${HOPS4_INSTALL_DIR}

Use the command ``ccmake`` (CMake curses GUI) to configure options manually, and to set paths to dependencies which may not be detected automatically.
On the first run, press 'c' to configure the build, 'e' to exit and edit, then 'c' to re-configure, and then 'g' to generate.

For example, the options table provided by ``ccmake`` will look something like the following:

::

  BASH_PROGRAM                    */usr/bin/bash
  BC_PROGRAM                      */usr/bin/bc
  CMAKE_BUILD_TYPE                *Release
  CMAKE_INSTALL_PREFIX            */home/oper/HOPS/x86_64-4.0.3
  CPGPLOT_LIBRARY                 */usr/lib/libcpgplot.so
  EXTRA_WARNINGS                  *OFF
  GFORTRAN_LIB                    */lib/x86_64-linux-gnu/libgfortran.so.5
  GS_EXE                          */usr/bin/gs
  HOPS3_DISABLE_WARNINGS          *ON
  HOPS3_PYTHON_EXTRAS             *ON
  HOPS3_USE_ADHOC_FLAGGING        *ON
  HOPS_BUILD_DOCS                 *OFF
  HOPS_BUILD_EXTRA_CONTAINERS     *OFF
  HOPS_CACHED_TEST_DATADIR        */home/oper/HOPS/x86_64-4.0.3/data/test_data
  HOPS_DEPLOY_DOCS                *OFF
  HOPS_ENABLE_COLOR_MSG           *ON
  HOPS_ENABLE_DEBUG_MSG           *ON
  HOPS_ENABLE_DEV_TODO            *OFF
  HOPS_ENABLE_EXTRA_VERBOSE_MSG   *OFF
  HOPS_ENABLE_SNAPSHOTS           *OFF
  HOPS_ENABLE_STEPWISE_CHECK      *OFF
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

Environment Setup Helpers
--------------------------

HOPS installs a few shell helpers in ``<hops-install>/bin/`` that set up environment variables.
They are all meant to be **sourced** (not executed), and only ``hops.bash`` is needed for normal use
of the installed tools:

.. list-table::
   :header-rows: 1
   :widths: 20 40 40

   * - Script
     - When to source it
     - What it sets
   * - ``hops.bash``
     - Always, to use HOPS
     - Adds HOPS to ``PATH``; sets ``HOPS_INSTALL`` and the other ``HOPS_*`` variables.
   * - ``hops_pypath.sh``
     - Only to ``import`` the HOPS Python modules/bindings for your **own** interpreter (interactive use or ad-hoc scripts)
     - Appends the HOPS site-packages dir to ``PYTHONPATH``.
   * - ``hops_buildenv.sh``
     - Only to **compile/link** your own code against an installed HOPS
     - ``CMAKE_PREFIX_PATH`` (for ``find_package(Hops)``) and ``PKG_CONFIG_PATH`` (for ``hops3.pc``/``hops4.pc``).

The two optional helpers rely on ``HOPS_INSTALL`` being set, so ``hops.bash`` must be sourced first.
The installed HOPS command-line tools self-locate their libraries and Python modules, so they do
**not** require ``hops_pypath.sh`` or ``hops_buildenv.sh``.

To ``import hops``, or ``import pyMHO_Containers``, etc. from an interpreter that you launch yourself,
source the ``PYTHONPATH`` helper after ``hops.bash``:

.. code-block:: bash

   source <hops-install>/bin/hops.bash
   source <hops-install>/bin/hops_pypath.sh

.. note::

   **Python interpreter pinning:** the pybind11 bindings (``pyMHO_*``) are compiled against the exact
   Python *minor* version found at configure time and are tagged accordingly (e.g.
   ``...cpython-310-...so``); they can only be imported by that same interpreter. Sourcing
   ``hops_pypath.sh`` adds the HOPS modules to ``PYTHONPATH``, which applies to **every** Python you run
   in that shell. Avoid mixing interpreters in a HOPS-sourced shell, if you activate a different
   environment (e.g. a ``conda`` env on a different Python version), its ``python`` will pick up the
   HOPS modules from ``PYTHONPATH`` and may fail to import the bindings if it is a different
   interpreter. Either use the interpreter HOPS was built against, or reconfigure/rebuild HOPS against
   the interpreter you intend to use.

When you want to compile a downstream project against an installed HOPS (e.g. via CMake
``find_package(Hops)`` or ``pkg-config``), source the build environment helper after ``hops.bash``:

.. code-block:: bash

   source <hops-install>/bin/hops.bash
   source <hops-install>/bin/hops_buildenv.sh

This adds the install prefix to ``CMAKE_PREFIX_PATH`` and the HOPS ``lib/pkgconfig`` directory to
``PKG_CONFIG_PATH``. Then from your project's ``CMakeLists.txt`` you can do something like:

.. code-block:: cmake

   find_package(Hops CONFIG REQUIRED)
   target_link_libraries(my_target PRIVATE Hops::MHO_Containers Hops::MHO_Utilities)

Alternatively, you can always just point CMake at your local install prefix directly without sourcing
the helper, e.g. ``cmake -DCMAKE_PREFIX_PATH=<hops-install> ..``. If you prefer to use pkg-config in
some other build system, typical library and compile flags can be extracted via:

.. code-block:: bash

   pkg-config --cflags --libs hops4
   pkg-config --cflags --libs hops3

Dependencies
------------

**Tip:** instead of installing the dependencies below by hand, you can run the helper script
``source/bash_src/hops-install-deps.sh``, which auto-detects the Linux package manager
(``apt``/``dnf``/``yum``) and installs the full HOPS4 + HOPS3 build and runtime dependencies for you
(use ``hops-install-deps.sh --help`` to see options such as ``--dry-run`` and ``--no-python``). The
script is also installed into ``<hops-install>/bin/``, in case you need to install additional
dependencies later to enable further options.

Required Dependencies
~~~~~~~~~~~~~~~~~~~~~

The following are required to build HOPS4:

.. list-table::
   :header-rows: 1
   :widths: 20 18 62

   * - Dependency
     - Version
     - Notes
   * - CMake
     - >= 3.8
     - Build system. ``cmake-curses-gui`` (``ccmake``) is recommended for interactive configuration.
   * - C++ compiler
     - C++11 (gcc >= 4.8.5 or clang >= 3.6)
     - C++17 is required when ``HOPS_USE_MATPLOTPP=ON`` (the default).
   * - Python 3
     - any
     - Required to build the Python bindings and post-processing scripts.
   * - pip
     - any
     - Required to install Python package dependencies.
   * - numpy
     - any
     - Python package; see ``HOPS_PYPI_MANAGE_DEPS`` below, needed by python scripts and python plotting backend).
   * - matplotlib
     - any
     - Python package (needed by python scripts and python plotting backend).
   * - scipy
     - any
     - Python package (needed by python scripts).
   * - wget, jq
     - any
     - Not required to build, but used by the test suite.

To install the build tools and system libraries on Ubuntu/Debian:

.. code-block:: bash

   sudo apt-get install build-essential cmake cmake-curses-gui python3-dev python3-pip wget jq

On RHEL/Fedora:

.. code-block:: bash

   sudo dnf install gcc-c++ cmake cmake-gui python3-devel python3-pip wget jq

Some RHEL-based distributions may require additional package repositories:

.. code-block:: bash

   sudo dnf config-manager --set-enabled crb
   dnf install epel-release

The Python packages (numpy, matplotlib, scipy) can be installed via pip.
If the CMake flag ``HOPS_PYPI_MANAGE_DEPS`` is set to ``ON``, pip will automatically download and locally install these packages into the HOPS install
directory at ``make install`` time. If set to ``OFF`` (the default), you are responsible for managing them yourself in the active python environment.
Note that if these packages are already installed system-wide and ``HOPS_PYPI_MANAGE_DEPS=ON``, a package conflict may result, but you can use a python virtual environment (venv) to avoid this.

Optional Dependencies
~~~~~~~~~~~~~~~~~~~~~

The following dependencies are optional (not required to build HOPS4, but associated features will be missing).
Each is controlled by a CMake build flag configurable via ``ccmake``.

.. list-table::
   :header-rows: 1
   :widths: 16 22 10 52

   * - Dependency
     - CMake Flag
     - Default
     - Notes
   * - FFTW3
     - ``HOPS_USE_FFTW3``
     - ON
     - Accelerates fringe fitting with optimized FFT routines. Highly recommended. Auto-disabled if not found.
   * - gnuplot
     - ``HOPS_USE_MATPLOTPP``
     - ON
     - Required runtime backend for matplot++ (fast static fringe plots). ``HOPS_USE_MATPLOTPP`` is disabled if gnuplot is not found.
   * - OpenMP
     - ``HOPS_USE_OPENMP``
     - ON
     - Shared-memory parallelism for the MBD search loop. Auto-disabled if not found.
   * - pybind11
     - ``HOPS_USE_PYBIND11``
     - ON
     - Python/C++ bindings. Requires Python development headers. A compatible version is bundled (see below).
   * - HDF5
     - ``HOPS_USE_HDF5``
     - OFF
     - Enables the ``hops2hdf5`` data export application.
   * - DiFXIO
     - ``HOPS_USE_DIFXIO``
     - OFF
     - Enables the ``difx2hops`` converter. Located via ``pkg-config``.
   * - MPI
     - ``HOPS_USE_MPI``
     - OFF
     - Enables distributed-memory parallel processing.
   * - OpenCL
     - ``HOPS_USE_OPENCL``
     - OFF
     - GPU acceleration via the OpenCL C++ wrapper API.
   * - CUDA
     - ``HOPS_USE_CUDA``
     - OFF
     - GPU acceleration via NVIDIA CUDA.

FFTW3 and gnuplot can be installed on Ubuntu/Debian with:

.. code-block:: bash

   sudo apt-get install libfftw3-dev gnuplot

Or on RHEL/Fedora:

.. code-block:: bash

   sudo dnf install fftw-devel gnuplot

Bundled Third-Party Libraries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HOPS4 includes several third-party libraries under ``extern/``. These are built and installed automatically.
No separate installation step is required by the user.

**Header-only libraries** are included at compile time and impose no additional runtime dependencies. These are:

.. list-table::
   :header-rows: 1
   :widths: 22 10 68

   * - Library
     - Version
     - Purpose
   * - `nlohmann/json <https://github.com/nlohmann/json>`_
     - 3.10.5
     - JSON serialization and deserialization, this is used throughout the HOPS4 data pipeline and file formats.
   * - `Eigen <https://eigen.tuxfamily.org>`_
     - 3.4.0
     - Linear algebra library used in some fringe fitting computations.
   * - `CLI11 <https://github.com/CLIUtils/CLI11>`_
     - 2.4.1
     - Command-line argument parsing for all HOPS4 applications.
   * - `pybind11_json <https://github.com/pybind/pybind11_json>`_
     - Captured copy under <extern>
     - Header-only bridge between pybind11 Python (dict) objects and nlohmann::json containers.

**Compiled bundled libraries** are built as part of HOPS4 and installed alongside the HOPS4 shared libraries. These are:

.. list-table::
   :header-rows: 1
   :widths: 22 10 68

   * - Library
     - Version
     - Purpose
   * - `pybind11 <https://github.com/pybind/pybind11>`_
     - 2.12 (captured copy under <extern>)
     - Python/C++ bindings; compiled when ``HOPS_USE_PYBIND11=ON``.
   * - `Howard Hinnant's date <https://github.com/HowardHinnant/date>`_
     - Captured copy under <extern>
     - Date and time library with IANA timezone database support (``tz`` shared library).
   * - `matplot++ <https://github.com/alandefreitas/matplotplusplus>`_
     - Captured copy under <extern>
     - C++ data visualization library; compiled when ``HOPS_USE_MATPLOTPP=ON``. However, this **does** require gnuplot to be installed by the user as the runtime backend.


HOPS3 Requirements
~~~~~~~~~~~~~~~~~~

If you also wish to build the legacy HOPS3 software (which is used heavily by the test suite), these additional dependencies are required:

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Dependency
     - Notes
   * - FFTW3
     - Required (not optional) for HOPS3.
   * - PGPLOT
     - Plotting library.
   * - X11
     - Required for PGPLOT display.
   * - GNU Fortran
     - Required to compile HOPS3 Fortran sources.
   * - Ghostscript
     - Required for postscript output.
   * - GSL (GNU Scientific Library)
     - Required in order to build cohfit.
   * - ImageMagick (``montage``)
     - Required in order to build cohfit.
   * - Gnuplot
     - Required in order to build cohfit.

On Ubuntu/Debian:

.. code-block:: bash

   sudo apt-get install python3-dev python3-pip pgplot5 libgfortran5 libfftw3-dev libx11-dev \
                        gnuplot binutils libxpm-dev ghostscript ghostscript-x gsl-bin libgsl-dev \
                        libgslcblas0 imagemagick

On RHEL/Fedora:

.. code-block:: bash

   sudo dnf install python3-devel python3-pip gcc-gfortran fftw-devel libX11-devel gnuplot \
                    binutils libXpm-devel ghostscript gsl gsl-devel ImageMagick

RHEL/Fedora distributions do not ship a PGPLOT package in their default repositories, but it is
available from `RPM Fusion <https://rpmfusion.org/>`_ (the nonfree repository, which hosts packages
that cannot be included in EPEL/Fedora proper due to licensing). After enabling the RPM Fusion
nonfree repository for your distribution, PGPLOT can be installed with:

.. code-block:: bash

   sudo dnf install pgplot pgplot-devel

If RPM Fusion is not available for your system, PGPLOT will have to be built manually from source;
see the note in ``<hops-source>/doc/notes/pgplot.txt`` for more information.
