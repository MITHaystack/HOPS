[![test-ubuntu](https://github.com/MITHaystack/HOPS/actions/workflows/cmake-ubuntu-22.yml/badge.svg)](https://github.com/MITHaystack/HOPS/actions/workflows/cmake-ubuntu-22.yml)
[![test-ubuntu-latest](https://github.com/MITHaystack/HOPS/actions/workflows/cmake-ubuntu-latest.yml/badge.svg)](https://github.com/MITHaystack/HOPS/actions/workflows/cmake-ubuntu-latest.yml)
[![deploy-docs](https://github.com/MITHaystack/HOPS/actions/workflows/deploy-docs.yml/badge.svg)](https://github.com/MITHaystack/HOPS/actions/workflows/deploy-docs.yml)
![HOPS4 Coverage](doc/coverage_hops4.svg)
![HOPS3 Coverage](doc/coverage_hops3.svg)
# HOPS 4 - Haystack Observatory Post-processing System

HOPS (Haystack Observatory Post-processing System) is a software package for fringe-fitting, calibration and other post-correlation
tasks required to process VLBI (Very Long Baseline Interferometry) data. HOPS4 is ground-up re-write of the original HOPS3 code
written in C++ and python. This repository also contains the original HOPS(3) software which can be installed alongside HOPS4.
For further information see the documentation [webpage](https://mithaystack.github.io/HOPS/).

## Getting started

To get the latest code from the repository, clone into the main branch with:

`git clone https://github.com/MITHaystack/HOPS.git`

Alternatively, tagged releases are available as .tar.gz archives from the release page [webpage](https://github.com/MITHaystack/HOPS/releases)

To configure the build system (accepting the default options) and compile the code using cmake, run the following:

`cd HOPS` \
`mkdir -p build && cd build && cmake .. && make && make install`

If you want faster compilation it is preferable to paralellize it by passing the `-j N` option to `make` (e.g. `make -j 8`).
After the build is complete, you will need to put the hops binary installation location in your PATH by running:

`source <hops-install>/bin/hops.bash`

where `<hops-install>` is the HOPS4 installation directory. On the x86 architecture, the default installation directory name
will be one level up from the build directory in: `<build-dir>/../x86_64-4.00`, but this can be changed at configuration time.
To set the install prefix to a custom path on the command line using `cmake`, you can pass the `-DCMAKE_INSTALL_PREFIX` flag as in the following example:

`HOPS4_INSTALL_DIR="~/hops-install"` \
`cmake -DCMAKE_INSTALL_PREFIX=${HOPS4_INSTALL_DIR}`

To change the default options or if a needed library is not automatically detected you can
use the command line interface `ccmake` (cmake-curses-gui) in place of `cmake` to edit the options. This command will open a CLI
GUI where you may change various parameters and toggle various optional dependencies. On the first run of this command you will be
presented with an "empty cache", so press `c` to configure, and then `e` exit and edit the options. Once you are satisfied with
your option selection, press `c` to configure again, and if no errors are detected, you can then generate the build system and
exit to the command line by pressing `g`. Now that the build system has been generated, run `make && make install` to compile
and install the software.

A full description of the configurable options can be found here:

[webpage](https://mithaystack.github.io/HOPS/ FIXME!)

## Dependencies/pre-requisites:

HOPS is organized into three main components, each with its own dependency set: the **HOPS4** C/C++ core (`source/cpp_src`),
the **HOPS4 Python tools** (`source/python_src`), and the legacy **HOPS3** suite (`source/c_src`, e.g. `fourfit3`).
The tables below describe the dependencies of each component and whether it is required or optional.
A number of libraries (Eigen, nlohmann/json, CLI11, pybind11, matplot++, and the date/tz library) which are used by HOPS4
are **bundled** in `extern/` and require no action from the end user to enable/use.

**Tip:** instead of installing the packages listed below by hand, you can try running the helper script
[`source/bash_src/hops-install-deps.sh`](source/bash_src/hops-install-deps.sh), which
auto-detects the Linux package managers `apt`/`dnf`/`yum` and will install the full HOPS4 + HOPS3 build and runtime
dependencies for you (use `hops-install-deps.sh --help` to see options such as `--dry-run`
and `--no-python`). The script is also installed into `<hops-install>/bin/`, in case you wish to run it in order to install  
dependencies needed for enabling additional options later. PGPLOT (a required dependency of HOPS3, not HOP4)
on RHEL/Fedora requires the RPM Fusion repository (see the PGPLOT note further below).

Dependencies can be managed directly as follows:  

### Build toolchain (required for any component)

| Dependency | Required? | Notes |
|---|---|---|
| CMake (>= 3.8), GNU make, bash | required | core build system |
| C++ compiler | required | C++11 minimum (gcc >= 4.8.5 / clang >= 3.6); C++17 if matplot++ plotting is enabled |
| cmake-curses-gui (`ccmake`) | optional | only for the interactive option-editing TUI |
| wget, jq | optional | only needed if you plane to download/parse test data for `make test` |

On Ubuntu 22.04 or Debian based systems these can be installed with:
```
sudo apt-get install build-essential cmake cmake-curses-gui wget jq
```
On RHEL/Fedora based distributions use:
```
sudo dnf install gcc-c++ cmake cmake-gui wget jq
```
Depending on your exact flavor of RHEL/Fedora, you may first need to enable the
`crb` repository and EPEL so that some of these packages can be located:
```
sudo dnf config-manager --set-enabled crb
sudo dnf install epel-release
```

### HOPS4 (C/C++ core + Python tools)

The HOPS4 core builds against only the bundled `extern/` libraries and the C++ standard library;
everything else is optional and can be toggled with a `cmake` option, as follows:

| Dependency | Required? | cmake option (default) | Purpose |
|---|---|---|---|
| C++ compiler + bundled externs | required | — | the core has no external library requirement |
| FFTW3 | optional (recommended) | `HOPS_USE_FFTW3` (ON) | accelerated FFTs; internal fallback used if OFF |
| Python 3 + dev headers | optional | `HOPS_USE_PYBIND11` (ON) | Python bindings and user plugins |
| gnuplot | optional (recommended) | `HOPS_USE_MATPLOTPP` (ON) | fast C++ fringe plotting; needs a C++17 capable compiler, auto-disabled if absent |
| OpenMP | optional | `HOPS_USE_OPENMP` (ON) | shared-memory parallelism; auto-disabled if absent |
| difxio | optional | `HOPS_USE_DIFXIO` (OFF) | read/import DiFX correlator output (`difx2hops`) |
| HDF5 | optional | `HOPS_USE_HDF5` (OFF) | HDF5 export |
| MPI / OpenCL / CUDA | optional | `HOPS_USE_MPI` / `_OPENCL` / `_CUDA` (OFF) | distributed / experimental GPU acceleration |

The **HOPS4 Python tools** (and the HOPS3 Python extras) require **numpy**, **matplotlib**, and **scipy** at runtime.
If you enable `-DHOPS_PYPI_MANAGE_DEPS=ON`, and have python-pip installed then pip will download and install these
dependencies *locally* into the HOPS install directory; If you prefer to manage your python dependencies separately, leave
this option set to (`OFF`). The Python tools will still be installed, but will require these packages to
be present in your Python environment in order to function at runtime.

Recommended HOPS4 extras on Ubuntu/Debian:
```
sudo apt-get install python3-dev python3-pip libfftw3-dev gnuplot
```
or on RHEL/Fedora:
```
sudo dnf install python3-devel python3-pip fftw-devel gnuplot
```

If you wish to build the original HOPS3 software suite (`fourfit3`, `fplot`,
`aedit`, `search`, `cohfit`, etc.) in addition to HOPS4, you will need several
additional dependencies:

| Dependency | Required? | Used by |
|---|---|---|
| PGPLOT | required | fourfit3, fplot, aedit, search, cohfit (plotting) |
| FFTW3 | required | fourfit3 and the fringe search |
| X11 | required | PGPLOT interactive display |
| GNU Fortran (libgfortran) | required | PGPLOT (Fortran library) |
| Ghostscript | required (runtime) | fringe-plot display/conversion |
| GSL | required | cohfit |
| ImageMagick (`montage`) | required | cohfit |
| Gnuplot | required | cohfit |
| Python 3 + dev headers | optional | HOPS3 Python (ctypes) extras (`HOPS3_PYTHON_EXTRAS`, ON) |
| libPNG | optional (auto-detected) | if PGPLOT was built against libpng |
| difxio | optional | `difx2mark4` (`HOPS_USE_DIFXIO` + `HOPS3_ENABLE_BUILD_D2M4`) |

**NOTE:** The HOPS3 libraries `dfio` and `mk4util` (mark4 data access), as well as `vex`, and `msg`, have no external
dependencies and will be built/installed regardless of whether the other HOPS3 dependencies are present.

On Ubuntu 22.04 or Debian based systems these dependencies can be installed with:

```
sudo apt-get install python3-dev python3-pip pgplot5 libgfortran5 libfftw3-dev \
  libx11-dev libxpm-dev gnuplot binutils ghostscript ghostscript-x \
  libgsl-dev gsl-bin imagemagick
```
while on RHEL/Fedora based systems these can be installed using:
```
sudo dnf install python3-devel python3-pip gcc-gfortran fftw-devel libX11-devel \
  libXpm-devel gnuplot binutils ghostscript gsl-devel ImageMagick
```
RHEL/Fedora distributions do not ship a PGPLOT package in their default repositories, but it is available from
[RPM Fusion](https://rpmfusion.org/) (the nonfree repository, which hosts packages that cannot be included in EPEL/Fedora
proper due to licensing). After enabling the RPM Fusion nonfree repository for your distribution, PGPLOT can be installed with:
```
sudo dnf install pgplot pgplot-devel
```
If RPM Fusion is not available for your system, PGPLOT will have to be built manually from source, see the note in
`<hops-source>/doc/notes/pgplot.txt` for more information.

## Testing
If you wish to check the functionality of the installation, you can run `make test` from the build directory after successfully building the software.
However you will first need to download the relevant set of test data. If you do not have a cached copy of the test data, you
can retrieve it by running the script `testdata_download_all.sh` as detailed below. This will download the necessary tarballs and place them in the HOPS test data directory under `<hops-install>/data/test_data`. The overall process is:

```
`cd HOPS && mkdir -p build && cd build && cmake .. && make && make install`
source <hops-install>/bin/hops.bash
testdata_download_all.sh
make test
```

If you have have chosen the default install directory then sourcing `hops.bash` (from the build directory):

`source ../x86_64-4.0.0/bin/hops.bash`

should print out the full path to the install directory, e.g:

`HOPS install directory set to /home/oper/HOPS/x86_64-4.0.0`

### Building the documentation
HOPS supports the ability to automatically build documentation using doxygen and sphinx. To do this, ensure that `doxygen`, `sphinx`, and the python packages `breathe` and `myst_parser` are installed, and that the cmake option `HOPS_BUILD_DOCS` is set to `ON`. To build and install the auto-generated documentation run the command `make reference && make install` from the build directory. The resulting html documentation will be placed in `<hops-install>/doc/reference`. The master index file will be installed as `<hops-install>/doc/reference/index.html` and can be opened with any browser.

### Getting help

For questions/comments on this software please direct emails to the developer mailing list: hops-dev@mit.edu

### License and Authorship
For license and authorship information see the LICENSE.md and AUTHORS.md files respectively.
