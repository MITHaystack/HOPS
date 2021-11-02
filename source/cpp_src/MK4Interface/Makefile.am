#
# Makefile for C++ Mk4 Interface Class
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

jsoninc = -I$(abs_top_srcdir)/extern/json/include
cxxmho = \
 	-I$(srcdir)/../Containers/include \
	-I$(srcdir)/../Utilities/include \
 	-I$(srcdir)/../Message/include \
 	-I$(srcdir)/../vex2json/include $(jsoninc)
cxxhops = \
	-I$(srcdir)/../../c_src/mk4util/include \
	-I$(srcdir)/../../c_src/dfio/include \
	-I$(srcdir)/../../c_src/vex/include
AM_CXXFLAGS = -I$(srcdir)/include $(MHO_CXXFLAGS) $(cxxhops) $(cxxmho)

pkginclude_HEADERS = \
	include/MHO_MK4Interface.hh \
	include/MHO_MK4CorelInterface.hh \
	include/MHO_MK4VexInterface.hh \
	include/MHO_MK4StationInterface.hh

pkglib_LTLIBRARIES = libMHO_MK4Interface.la

libMHO_MK4Interface_la_SOURCES = \
	src/MHO_MK4Interface.cc \
	src/MHO_MK4CorelInterface.cc \
	src/MHO_MK4StationInterface.cc \
	src/MHO_MK4VexInterface.cc
libMHO_MK4Interface_la_LDFLAGS = -avoid-version

TESTS = TestMK4Import TestMK4StationImport TestMK4VexInterface
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)
XFAIL_TESTS = TestMK4Import TestMK4StationImport

check_PROGRAMS = $(TESTS)

thislib = -L.libs -lMHO_MK4Interface
message = -L../Message/.libs -lMHO_Message
utilities = -L../Utilities/.libs -lMHO_Utilities
vexables = -L../vex2json/.libs -lvex2json
mholibs = $(message) $(utilities)
hopslibs = -L../../c_src/dfio -ldfio \
	-L../../c_src/vex -lvex \
	-L../../c_src/mk4util -lmk4util \
	-L../../c_src/msg -lmsg
TestMK4Import_SOURCES = test/TestMK4Import.cc
#TestMK4Import_CXXFLAGS =  $(AM_CXXFLAGS)
TestMK4Import_LDADD = $(thislib) $(mholibs) $(hopslibs)
TestMK4StationImport_SOURCES = test/TestMK4StationImport.cc
TestMK4StationImport_LDADD = $(thislib) $(mholibs) $(hopslibs)
TestMK4VexInterface_SOURCES = test/TestMK4VexInterface.cc
TestMK4VexInterface_LDADD = $(thislib) $(mholibs) $(vexables) $(hopslibs)

CLEANFILES =

EXTRA_DIST = CMakeLists.txt test/CMakeLists.txt

#
# eof
#
