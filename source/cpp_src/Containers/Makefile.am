#
# Makefile for C++ Container Class
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

cxxmho = -I$(srcdir)/../Message/include -I$(srcdir)/../Utilities/include
AM_CXXFLAGS = -I$(srcdir)/include $(cxxmho) $(MHO_CXXFLAGS)

pkginclude_HEADERS = \
	include/MHO_Axis.hh \
	include/MHO_AxisPack.hh \
	include/MHO_ChannelizedVisibilities.hh \
	include/MHO_ContainerDictionary.hh \
	include/MHO_NDArrayMath.hh \
	include/MHO_NDArrayTypeCastFunctor.hh \
	include/MHO_NDArrayWrapper_0.hh \
	include/MHO_NDArrayWrapper_1.hh \
	include/MHO_NDArrayWrapper.hh \
	include/MHO_ScalarContainer.hh \
	include/MHO_StationCoordinates.hh \
	include/MHO_TableContainer.hh \
	include/MHO_VectorContainer.hh \
	include/MHO_Visibilities.hh \
	include/MHO_VisibilityType.hh

pkglib_LTLIBRARIES = libMHO_Containers.la

libMHO_Containers_la_SOURCES = src/MHO_ContainerDictionary.cc
libMHO_Containers_la_LDFLAGS = -avoid-version

TESTS = TestArrayWrapper TestVectorContainer TestScalarContainer \
	TestTableContainer TestAxis TestAxisPack
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

check_PROGRAMS = $(TESTS)

thislib = -L.libs -lMHO_Containers
message = -L../Message/.libs -lMHO_Message
utilities = -L../Utilities/.libs -lMHO_Utilities
mholibs = $(thislib) $(message) $(utilities)
TestArrayWrapper_SOURCES = test/TestArrayWrapper.cc
TestArrayWrapper_LDADD = $(mholibs)
TestVectorContainer_SOURCES = test/TestVectorContainer.cc
TestVectorContainer_LDADD = $(mholibs)
TestScalarContainer_SOURCES = test/TestScalarContainer.cc
TestScalarContainer_LDADD = $(mholibs)
TestTableContainer_SOURCES = test/TestTableContainer.cc
TestTableContainer_LDADD = $(mholibs)
TestAxis_SOURCES = test/TestAxis.cc
TestAxis_LDADD = $(mholibs)
TestAxisPack_SOURCES = test/TestAxisPack.cc
TestAxisPack_LDADD = $(mholibs)

CLEANFILES = test.bin test-table.index test-table.bin test-axis.bin
CLEANFILES += test-axis-pack.bin

EXTRA_DIST = CMakeLists.txt test/CMakeLists.txt

#
# eof
#
