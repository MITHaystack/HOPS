# HOPS-4.0
## MSRI HOPS re-development
John Barrett, Geoff Crew, Dan Hoak, Violet Pfeiffer

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

(1) Python 3.x \
(2) FFTW3 \
(3) PGPLOT \
(4) X11 \
(5) GFORTRAN
