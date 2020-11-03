#
# Makefile for python bindings to C/C++
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
# This directory supports compilation of a SWIG-based interface library
# which allows python to call C/C++ functionality.  include/*.i are
# interface specifications for SWIG to read to generate .c files for
# this library.  Additional src/*.c src/*.cc files supply "glue" to
# simplify the interface.
#
# The include/numpy.i interface is obtained from github/numpy
#

#AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall
#pkginclude_HEADERS = include/MHOXXX.hh
#pkglib_LTLIBRARIES = libMHOXXX.la
#libMHOXXX_la_SOURCES = src/MHOUXXX.cc
#libMHOXXX_la_LDFLAGS = -avoid-version

#pyhops.c: $(srcdir)/include/pyhops.i 



# one-time import rule
githuburl = https://raw.githubusercontent.com
numpy_master_swigs = \
	$(srcdir)/include/numpy.i \
	$(srcdir)/include/numpy-README.txt \
	$(srcdir)/include/numpy-LICENSE.txt
one-time-import: $(numpy_master_swigs)
$(srcdir)/include/numpy.i:
	wget $(githuburl)/numpy/numpy/master/tools/swig/numpy.i
	mv numpy.i $(srcdir)/include/numpy.i
$(srcdir)/include/numpy-README.txt:
	wget $(githuburl)/numpy/numpy/master/tools/swig/README
	mv README $(srcdir)/include/numpy-README.txt
$(srcdir)/include/numpy-LICENSE.txt:
	wget $(githuburl)/numpy/numpy/master/LICENSE.txt
	mv LICENSE.txt $(srcdir)/include/numpy-LICENSE.txt

EXTRA_DIST = $(numpy_master_swigs)

clean-local:
	-rm -rf __pycache__
#
# eof
#
