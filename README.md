[![test-ubuntu](https://github.com/MITHaystack/HOPS/actions/workflows/cmake-ubuntu.yml/badge.svg)](https://github.com/MITHaystack/HOPS/actions/workflows/cmake-ubuntu.yml)
# HOPS 4 - Haystack Observatory Post-processing System

HOPS (Haystack Observatory Post-processing System) is a software package for fringe-fitting, calibration and other post-correlation tasks required to process VLBI (Very Long Baseline Interferometry) data. HOPS4 is ground-up re-write done in C/C++ and python which is an ongoing effort to create a more modular framework capable of easier future extension and modification. This repository also contains the original HOPS(3) software which can be installed alongside HOPS4. For further information see the release notes under `<hops-source>/doc/notes/release.txt`.

To configure the build system with the default options and compile the code using cmake, run the following:  

`$ cd <hops-source>` \
`$ mkdir build` \
`$ cd ./build` \
`$ cmake ../` \
`$ make && make install`

If you want a faster build it is preferable to paralellize it by passing the `-j N` option to `make` (e.g. `make -j 8`).
After the build is complete, you will need to put the hops binary installation location in your PATH by running:

`source <hops-install>/bin/hops.bash`

where `<hops-install>` is the HOPS4 installation directory. On the x86 architecture, the default installation directory name 
will be one level up from the build directory in: `<build-dir>/../x86_64-4.00`, but can be changed at configuration time. To set the install 
prefix to a custom path on the command line using `cmake`, you can pass the `-DCMAKE_INSTALL_PREFIX` flag as in the following example:

`HOPS4_INSTALL_DIR="~/hops-install"` \
`cmake -DCMAKE_INSTALL_PREFIX=${HOPS4_INSTALL_DIR}`

To change the default options or if a needed library is not automatically detected you can
use the command line interface `ccmake` (cmake-curses-gui) in place of `cmake` to edit options. This command will open a CLI
GUI where you may change various parameters. On the first run of this command you will need to presented with an "empty cache", so press 'c'
to configure, and then 'e' exit and edit the options. Once you are satisfied with your option selection, press 'c' to configure again. If no errors
are detected, you can then generate the build system and exit to the command line by pressing 'g'. Once the build system has been generated, then run `make && make install` to compile and install.

An example of the ccmake option table (with defaults) is shown below:
```
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
```

## Current pre-requisites:

The absolute minimum dependencies are:

1. cmake, cmake-curses-gui, GNU make, and bash
2. A c++ compiler which supports the C++11 standard (gcc >= 4.8.5, or clang >= 3.6)
3. python3 and pip (if you want to make use of python extensions)
4. For post-installation testing (e.g. make test) the utilities wget and jq are needed, but are not required for installation.

On Ubuntu 22.04 or Debian based systems these can be installed with:

```
sudo apt-get install build-essential cmake cmake-curses-gui python3-dev python3-pip wget jq
```
On RHEL/Fedora based distributions, these dependencies can be installed with:
```
sudo dnf install gcc-c++ cmake cmake-gui python3-devel python3-pip wget jq
```
Note that depending on your exact flavor of RHEL/Fedora you may need to first enable the following:
```
sudo dnf config-manager --set-enabled crb
dnf install epel-release
```
So that you can locate some of these packages.
While not strictly required by HOPS4, the Fast Fourier Transform library fftw is highly recommended and can be installed with:
```
sudo apt-get install libfftw3-dev
```
or 
```
sudo dnf install fftw-devel
```

Python package installation is handled by pip, but it will not pull in any python dependencies (numpy, matplotlib, scipy, future) unless the user explicitly enables the cmake flag `HOPS_PYPI_MANAGE_DEPS`, if this option is set to 'ON' then pip with attempt to download and *locally* install the necessary python packages in the HOPS install directory. If you prefer to manage you own python dependencies, then leave this option set to 'OFF', in this case the HOPS python tools will still be installed, but only be capable of running if the necessary dependencies are found in the users python environment.

If you wish to build the original HOPS3 software suite (fourfit, etc.), in addition to HOPS4, you will need
several additional dependencies, these are:

1. `Python 3.x`
2. `FFTW3`
3. `PGPLOT`
4. `X11`
5. `GNU Fortran`
6. `Ghostscript`

On Ubuntu 22.04 or Debian based systems these can be installed with:

```
sudo apt-get install python3-dev python3-pip pgplot5 libgfortran5 libfftw3-dev libx11-dev gnuplot binutils libx11-dev libxpm-dev ghostscript ghostscript-x
```
while on RHEL/Fedora based systems these can be installed using:
```
sudo dnf install python3-devel python3-pip gcc-gfortran fftw-devel libX11-devel gnuplot binutils libXpm-devel ghostscript
```
However, RHEL/Fedora disributions lack a package for the PGPLOT library needed by HOPS3, so this will have to be installed manually 
from source on these distributions, see the note in `<hops-source>/doc/notes/pgplot.txt` for more information regarding PGPLOT.

## Testing
If you wish to check the functionality of the installation, you can run `make test` from the build directory after successfully building the software.
However you will first need to download the relevant set of test data. If you do not have a cached copy of the test data, you
can retrieve it by running the script `testdata_download_all.sh` as detailed below. This will download the necessary tarballs and place them in the HOPS test data directory under `<hops-install>/data/test_data`. The overall process is:

```
cd <hops-git>
mkdir ./build
cd ./build
make
make install
source <hops-install>/bin/hops.bash
testdata_download_all.sh
make test
```

If you have have chosen the default install directory then sourcing `hops.bash` (from the build directory):

`source ../x86_64-4.0.0/bin/hops.bash`

should print out the full path to the install directory:

`HOPS install directory set to /home/oper/HOPS/x86_64-4.0.0`

Do not worry about messages concerning an undefined 'CI/CD key', this is for downloading secured test data, tests that are related to that data will be skipped.
You will need a terminal at least 95 characters wide to avoid line wraps from the `make test` suite output. 

### Building the documentation
HOPS supports the ability to automatically build documentation using doxygen and sphinx. To do this, ensure that `doxygen`, `sphinx`, and the python packages `breathe` and `myst_parser` are installed, and that the cmake option `BUILD_DOCS` is set to `ON`. To build and install the auto-generated documentation run the command `make reference && make install` from the build directory. The resulting html documentation will be placed in `<hops-install>/doc/reference`. The master index file will be installed as `<hops-install>/doc/reference/index.html` and can be opened with any browser.

### Getting help

For questions/comments on this software please direct emails to the developer mailing list: hops-dev@mit.edu

### License and Authorship 
For license and authorship information see the LICENSE.md and AUTHORS.md files respectively.
