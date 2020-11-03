# HOPS-4.0
## MSRI HOPS re-development
John Barrett, Geoff Crew, Dan Hoak, Violet Pfeiffer

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

## Current pre-requisites:

(1) cmake, cmake-curses-gui, GNU make, and bash \
(2) A c++ compiler which supports the C++11 standard (gcc > 4.8, or clang > 3.3)

Optional dependencies (these may possibly become required at some point in the future):

(1) `Python 3.x` \
(2) `FFTW3` \
(3) `PGPLOT` \
(4) `X11` \
(5) `GFORTRAN`

## Checking the distribution
Currently HOPS supports the ability to run `make distcheck` from the build directory assuming `make` was used to build the project instead of `cmake`.
`distcheck` is not currently compatible with `cmake`. `distcheck` checks that the tarball distribution is in working order and all the necessary files are included.

## Documentation Dependencies
### Ubuntu packages
(1) `texlive-full` \
(2) `graphviz` \
(3) `doxygen`
### Fedora packages
### NixOS packages
(1) `texlive.combined.scheme-full` \
(2) `graphviz` \
(3) `doxygen` 

An equivalent build process using the GNU autotools is also available
using the script autogen.sh which accepts a --help argument for usage.
