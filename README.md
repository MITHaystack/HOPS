# HOPS-4.0
## MSRI HOPS re-development
John Barrett, Geoff Crew, Dan Hoak, Violet Pfeiffer

Mailing list: hops-dev@mit.edu

clone from: https://github.mit.edu/barrettj/hops-git.git

To configure with defaults and compile with cmake, run:

`$ cd ./chops` \
`$ mkdir build` \
`$ cd ./build` \
`$ cmake ../` \
`$ make && make install`

To change the default options or if a library is not automatically found and you
need to specify a directory path in order for it to be located, use the command
line interface ccmake (cmake-curses-gui) in place of cmake. This will open a CLI
GUI where you may change various parameters.

## Alternate build with Automake

An alternate build path is available via the GNU auto build tools (autoconf, automake, etc.).  A top-level build script autogen.sh (which takes --help) can be used to build everything.  As usual with autoconfigured scripts, environment variables may need to be set to get it all to work.  For example:

`$ ./autogen.sh --help` \
`$ HOPS_CONFIGURE_ARGS='CXXFLAGS=-std=c++11 --enable-gcov' \` \
`  ./autogen.sh true false false centos-7` \
`$ HOPS_CONFIGURE_ARGS='--enable-gcov' \` \
`  ./autogen.sh true false false fedora-33`

(The true/false directives control what the script does, and the last
argument is appended to the name of the build directory--here we use it
to indicate what was needed on two Red Hat flavor OSs.)

## Current pre-requisites:

(1) cmake, cmake-curses-gui, GNU make, and bash \
(2) A c++ compiler which supports the C++11 standard (gcc > 4.8, or clang > 3.3)

Optional dependencies (it depends on how much of HOPS you want to build):

(1) `Python 3.x` \
(2) `FFTW3` \
(3) `PGPLOT` \
(4) `X11` \
(5) `GNU Fortran`

## Checking the distribution
Currently HOPS supports the ability to run `make distcheck` from the build directory assuming `make` was used to build the project instead of `cmake`.
`distcheck` is not currently compatible with `cmake`. `distcheck` checks that the tarball distribution is in working order and all the necessary files are included.

## Documentation Dependencies
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
HOPS supports the ability to build the documentation as well as the code with `make` by doing the following:
`cd hops-git/` \
`./autogen.sh` \
Copy the output of the configure script that looks something like configure=/some/path
Paste that in your shell and run it. \
`cd ../` \
`mkdir ambld-4.0` \
`cd ambld-4.0` \
`$configure --enable-devel --enable-docs CC=clang --enable-doxy` \
`make all check install`

An equivalent build process using the GNU autotools is also available
using the script autogen.sh which accepts a --help argument for usage.

## Comprehensive list of dependencies
| Dependency | Version | Required | Suggested | Ubuntu Package | Fedora Package | NixOS Package | Notes                              |
| -----------|---------|----------|-----------|----------------|----------------|---------------|------------------------------------|
| autoconf   | 2.69+   | yes      | N/A       | autoconf       |                |               |                                    |
| autoheader | 2.69+   | yes      | N/A       |                |                |               |                                    |
| autogen    | 5.18.16+| yes      | N/A       | autogen        |                |               |                                    |
| libtoolize | 2.4.6+  | yes      | N/A       | libtool        |                |               |                                    |
| aclocal    | 1.16.1+ | yes      | N/A       |                |                |               |                                    |
| fftw3      | 3       | No       | Yes       | libfftw3-3     |                |               |                                    |
| gfortran   | 9.3.0+  | No       | Yes       | gfortran       |                |               |                                    |
| PGPLOT     | 5.2     | No       | Yes       |                |                |               | Recommended to compile from source |
