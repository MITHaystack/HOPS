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
line interface ccmake (cmake-curses-gui) in place of cmake. This will open a CLI
GUI where you may change various parameters. An example of this (with some optional
dependencies turned on/off, e.g. OpenCL, PyBind11, etc.) is shown below:
```
 BASH_PROGRAM                     /usr/bin/bash                                                                                                                                                           
 BUILD_DOXYGEN_REF                OFF                                                                                                                                                                     
 BUILD_LATEX_DOCS                 OFF                                                                                                                                                                     
 CMAKE_BUILD_TYPE                                                                                                                                                                                         
 CMAKE_INSTALL_PREFIX             /home/barrettj/work/projects/hops-git/x86_64-4.0.0                                                                                                                      
 CPGPLOT_LIBRARY                  /usr/local/pgplot/libcpgplot.a                                                                                                                                          
 ENABLE_DEBUG_MSG                 OFF                                                                                                                                                                     
 GFORTRAN_LIB                     /usr/lib/gcc/x86_64-linux-gnu/9/libgfortran.so                                                                                                                          
 HOPS3_DISABLE_WARNINGS           ON                                                                                                                                                                      
 HOPS3_USE_CXX                    ON                                                                                                                                                                      
 HOPS_DEV_USE_OLD                 OFF                                                                                                                                                                     
 HOPS_ENABLE_REMOTE_TEST_DATA     OFF                                                                                                                                                                     
 HOPS_ENABLE_TEST                 ON                                                                                                                                                                      
 HOPS_USE_DIFXIO                  OFF                                                                                                                                                                     
 HOPS_USE_FFTW3                   ON                                                                                                                                                                      
 HOPS_USE_OPENCL                  ON                                                                                                                                                                      
 HOPS_USE_PYBIND11                OFF                                                                                                                                                                     
 HOPS_USE_ROOT                    OFF                                                                                                                                                                     
 LATEX_DEFAULT_BUILD              pdf                                                                                                                                                                     
 LATEX_OUTPUT_PATH                                                                                                                                                                                        
 LATEX_SMALL_IMAGES               OFF                                                                                                                                                                     
 LATEX_USE_SYNCTEX                OFF                                                                                                                                                                     
 OPENCL_LIBRARIES                 /usr/lib/x86_64-linux-gnu/libOpenCL.so                                                                                                                                  
 TAR_PROGRAM                      /usr/bin/tar                                                                                                                                                            
 WGET_PROGRAM                     /usr/bin/wget                                                                                                                                                           
 _OPENCL_CPP_INCLUDE_DIRS         /usr/include    
```



<!-- ## Alternate build with Automake

An alternate build path is available via the GNU auto build tools (autoconf, automake, etc.).  A top-level build script autogen.sh (which takes --help) can be used to build everything.  As usual with autoconfigured scripts, environment variables may need to be set to get it all to work.  For example:

`$ ./autogen.sh --help` \
`$ HOPS_CONFIGURE_ARGS='CXXFLAGS=-std=c++11 --enable-gcov' \` \
`  ./autogen.sh true false false centos-7` \
`$ HOPS_CONFIGURE_ARGS='--enable-gcov' \` \
`  ./autogen.sh true false false fedora-33`

(The true/false directives control what the script does, and the last
argument is appended to the name of the build directory--here we use it
to indicate what was needed on two Red Hat flavor OSs.) -->

## Current pre-requisites:

The absolute minimum dependencies are:

(1) cmake, cmake-curses-gui, GNU make, and bash \
(2) A c++ compiler which supports the C++11 standard (gcc > 4.8, or clang > 3.3)
(3) python3 and pip (if you want to make use of python extensions)
(4) For post-installation testing (e.g. make test) the utilities wget and jq are needed, but are not required for installation.

On Ubuntu 22.04 or Debian based systems these can be installed with:

```
sudo apt-get install build-essential cmake cmake-curses-gui
sudo apt-get install python3-dev python3-pip
sudo apt-get install wget jq
```

However, there are several optional dependencies which will enable additional features if they are present.
If you wish to build the original HOPS3 software suite (fourfit, etc.), you will need at a minimum:

(1) `Python 3.x` \
(2) `FFTW3` \
(3) `PGPLOT` \
(4) `X11` \
(5) `GNU Fortran`

On Ubuntu 22.04 or Debian based systems these can be installed with:

```
sudo apt-get install python3-dev python3-pip
sudo apt-get install pgplot5 libgfortran5
sudo apt-get install libfftw3-dev
sudo apt-get install libx11-dev
sudo apt-get install gnuplot
sudo apt-get install binutils libx11-dev libxpm-dev \
```

### Installing pgplot
* Otherwise compile and install from source with this script:
  `source/shell_src/install-pgplot.sh`



## Checking the distribution
If you wish to check the correctness of the installation, you can enable the option `HOPS_ENABLE_TEST` using `ccmake` before building the software (default is `OFF`).
Then after building you can run `make test` after running `make install` and `source <hops-install>/bin/hops.bash`.
However, if you do not have a cached copy of the test data, you must first install the pre-requisites `wget` and `jq`, and 
also ensure that the cmake option `HOPS_ENABLE_REMOTE_TEST_DATA` is set to `ON`, so that the test data tarballs can be 
retrieved and cached in the build directory (TODO -- add alternate instructions to obtain the test data).

### Testing
```
cd <hops-git>
mkdir build
cd build
```
Note: If this is the first time you are running tests you will need to turn `HOPS_ENABLE_TEST` on using `ccmake ../`.
```
ccmake ../
make && make build
source ../x86_64-4.00/bin/hops.bash
```
Then to run all tests:
```
make test
```


<!-- Currently HOPS supports the ability to run `make distcheck` from the build directory assuming `make` was used to build the project instead of `cmake`.
`distcheck` is not currently compatible with `cmake`. `distcheck` checks that the tarball distribution is in working order and all the necessary files are included. -->

<!-- ## Documentation Dependencies
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
using the script autogen.sh which accepts a --help argument for usage. -->
