#
# Makefile for C++ Mk4 Interface Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

pkginclude_HEADERS = include/MHO_MK4CorelInterface.hh \
	include/MHO_MK4FringeInterface.hh \
	include/MHO_MK4Interface.hh \
	include/MHO_MK4VexInterface.hh \
	include/MHO_MK4Visibility.hh

pkglib_LTLIBRARIES = libMHO_MK4Interface.la

check_PROGRAMS = TestMK4Import TestMK4VexInterface

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

cxxflags = -I. -I$(srcdir)/include -Wall -Wextra
# v4
cxxmho = -I$(srcdir)/../Utilities/include \
	-I$(srcdir)/../Containers/include \
	-I$(srcdir)/../Message/include
# v3 -- FIXME
cxxhops = -I$(srcdir)/../../c_src/mk4util/include \
	-I$(srcdir)/../../c_src/dfio/include \
	-I$(srcdir)/../../c_src/vex/include
AM_CXXFLAGS = $(cxxflags) $(cxxmho) $(cxxhops)

libMHO_MK4Interface_la_SOURCES = src/MHO_MK4CorelInterface.cc \
	src/MHO_MK4FringeInterface.cc \
	src/MHO_MK4Interface.cc \
	src/MHO_MK4VexInterface.cc
libMHO_MK4Interface_la_LDFLAGS = -avoid-version

# v4
thislib = -L.libs -lMHO_MK4Interface
message = -L../Message/.libs -lMHO_Message
utilities = -L../Utilities/.libs -lMHO_Utilities
mholibs = $(message) $(utilities)
# v3 -- FIXME
#hopslibs = -L../../c_src/mk4util -lmk4util \
#	-L../../c_src/dfio -ldfio \
#	-L../../c_src/vex -lvex
hopslibs = -L../../c_src/dfio -ldfio \
	-L../../c_src/vex -lvex \
	-L../../c_src/mk4util -lmk4util 

TestMK4Import_SOURCES = test/TestMK4Import.cc
TestMK4Import_LDADD = $(thislib) $(mholibs) $(hopslibs)

TestMK4VexInterface_SOURCES = test/TestMK4VexInterface.cc
TestMK4VexInterface_LDADD = $(thislib) $(mholibs) $(hopslibs)

EXTRA_DIST =

#
# eof
#
