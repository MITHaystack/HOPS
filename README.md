# HOPS-4.0
## MSRI HOPS re-development
John Barrett, Geoff Crew, Dan Hoak, Violet Pfeiffer

Mailing list: hops-dev@mit.edu

clone from: https://github.mit.edu/barrettj/hops-git.git

To configure with defaults and compile with cmake, run:

`$ cd <hops-git>` \
`$ mkdir build` \
`$ cd ./build` \
`$ cmake ../` \
`$ make && make install`

If you want a faster build it is preferable to paralellize it with the `-j N` option (e.g. `make -j 8`).
After the build is complete, you can put all of the hops executables in your PATH by running:

`source <hops-install>/bin/hops.bash`

where `<hops-install>` is the HOPS4 installation directory.

To change the default options or if a library is not automatically found and you
need to specify a directory path in order for it to be located, use the command
line interface `ccmake` (cmake-curses-gui) in place of `cmake`. This will open a CLI
GUI where you may change various parameters. An example of this (with some optional
dependencies turned on/off, e.g. OpenCL, PyBind11, etc.) is shown below:
```
 BASH_PROGRAM                     /usr/bin/bash
 BC_PROGRAM                       /usr/bin/bc
 BUILD_DOXYGEN_REF                ON
 BUILD_LATEX_DOCS                 OFF
 CMAKE_BUILD_TYPE                 RelWithDebInfo
 CMAKE_INSTALL_PREFIX             /home/barrettj/work/projects/hops-git/x86_64-4.00
 CPGPLOT_LIBRARY                  /usr/lib/libcpgplot.so
 ENABLE_COLOR_MSG                 ON
 ENABLE_DEBUG_MSG                 ON
 ENABLE_EXTRA_VERBOSE_MSG         OFF
 ENABLE_SNAPSHOTS                 OFF
 ENABLE_STEPWISE_CHECK            OFF
 EXTRA_WARNINGS                   OFF
 GFORTRAN_LIB                     /lib/x86_64-linux-gnu/libgfortran.so.5
 GS_EXE                           /usr/bin/gs
 HOPS3_DISABLE_WARNINGS           ON
 HOPS3_PYTHON_EXTRAS              ON
 HOPS3_USE_ADHOC_FLAGGING         ON
 HOPS3_USE_CXX                    OFF
 HOPS_BUILD_EXTRA_CONTAINERS      OFF
 HOPS_ENABLE_REMOTE_TEST_DATA     ON
 HOPS_ENABLE_TEST                 ON
 HOPS_PYPI_MANAGE_DEPS            ON
 HOPS_USE_DIFXIO                  ON
 HOPS_USE_FFTW3                   ON
 HOPS_USE_MPI                     OFF
 HOPS_USE_OPENCL                  OFF
 HOPS_USE_PYBIND11                ON
 HOPS_USE_ROOT                    OFF
 PYBIND11_FINDPYTHON              OFF
 PYBIND11_INSTALL                 OFF
 PYBIND11_INTERNALS_VERSION
 PYBIND11_NOPYTHON                OFF
 PYBIND11_SIMPLE_GIL_MANAGEMENT   OFF
 PYBIND11_TEST                    OFF
 TAR_PROGRAM                      /usr/bin/tar
 WGET_PROGRAM                     /usr/bin/wget
```

## Current pre-requisites:

The absolute minimum dependencies are:

1. cmake, cmake-curses-gui, GNU make, and bash
2. A c++ compiler which supports the C++11 standard (gcc > 4.8, or clang > 3.3)
3. python3 and pip (if you want to make use of python extensions)
4. For post-installation testing (e.g. make test) the utilities wget and jq are needed, but are not required for installation.

On Ubuntu 22.04 or Debian based systems these can be installed with:

```
sudo apt-get install build-essential cmake cmake-curses-gui
sudo apt-get install python3-dev python3-pip
sudo apt-get install wget jq
```

While not strictly required by HOPS4, the Fast Fourier Transform library fftw is highly recommended and can be installed with:
```
sudo apt-get install libfftw3-dev
```

Python package installation is handled by pip, but it will not pull in any python dependencies (numpy, matplotlib, scipy) unless the user enables the cmake flag `HOPS_PYPI_MANAGE_DEPS`,
if this option is turned on pip with attempt to download and locally install the necessary python packages.

If you wish to build the original HOPS3 software suite (fourfit, etc.), in addition to HOPS4, you will need
several additional dependencies, these are:

1. `Python 3.x` \
2. `FFTW3` \
3. `PGPLOT` \
4. `X11` \
5. `GNU Fortran`

On Ubuntu 22.04 or Debian based systems these can be installed with:

```
sudo apt-get install python3-dev python3-pip
sudo apt-get install pgplot5 libgfortran5
sudo apt-get install libfftw3-dev
sudo apt-get install libx11-dev
sudo apt-get install gnuplot
sudo apt-get install binutils libx11-dev libxpm-dev
```

### Installing pgplot
* If the pgplot5 package is missing you can otherwise compile and install pgplot from source with this script:
  `<hops-git>/installation/shell_src/install-pgplot.sh`


## Testing
If you wish to check the correctness of the installation, then after successfully building the software with:
```
cd <hops-git>
mkdir build
cd build
make
make install
source <hops-install>/bin/hops.bash
```
you can run `make test`.

Note that if you do not have a cached copy of the test data and the cmake option `HOPS_ENABLE_REMOTE_TEST_DATA` is set to `ON`, the test data
tarballs will be downloaded and cached in the build directory (TODO -- add alternate instructions to manually obtain the test data).

### Building the documentation
HOPS supports the ability to automatically build documentation using the doxygen too. To do this, ensure that doxygen is installed, and that the
cmake option `BUILD_DOXYGEN_REF` is set to `ON`. To build and install the auto-generated documentation run the command `make reference` from the build directory.
The resulting html documentation will be placed in `<hops-install>/doc/reference`. The master index file will be installed as `<hops-install>/doc/Hops.html` and can
be opened with any browser.


## Alternate build with Automake (not up to date)

An alternate build path is available via the GNU auto build tools (autoconf, automake, etc.).  A top-level build script autogen.sh (which takes --help) can be used to build everything.  As usual with autoconfigured scripts, environment variables may need to be set to get it all to work.  For example:

`$ ./autogen.sh --help` \
`$ HOPS_CONFIGURE_ARGS='CXXFLAGS=-std=c++11 --enable-gcov' \` \
`  ./autogen.sh true false false centos-7` \
`$ HOPS_CONFIGURE_ARGS='--enable-gcov' \` \
`  ./autogen.sh true false false fedora-33`

(The true/false directives control what the script does, and the last
argument is appended to the name of the build directory--here we use it
to indicate what was needed on two Red Hat flavor OSs.)

The HOPS automake build supports the ability to run `make distcheck` from the build directory assuming `make` was used to build the project instead of `cmake`.
`distcheck` is not currently compatible with `cmake`. `distcheck` checks that the tarball distribution is in working order and all the necessary files are included.

## Automake Documentation Dependencies
### Ubuntu packages
(1) `texlive-full` \
(2) `graphviz` \
(3) `doxygen` \
(4) `sphinx-common`
pkg-config, csh, autoconf, libtool, xorg, openbox, libx11-dev, fftw3, fftw3-dev
python3-sphinx, python3-dev, python-dev, swig, help2man


### Fedora packages
### NixOS packages
(1) `texlive.combined.scheme-full` \
(2) `graphviz` \
(3) `doxygen`

### Building the documentation
The automake build supports the ability to build the documentation as well as the code with `make` by doing the following:
`cd hops-git/` \
`./autogen.sh` \
Copy the output of the configure script that looks something like configure=/some/path
Paste that in your shell and run it. \
`cd ../` \
`mkdir ambld-4.0` \
`cd ambld-4.0` \
`$configure --enable-devel --enable-docs CC=clang --enable-doxy` \
`make all check install`
