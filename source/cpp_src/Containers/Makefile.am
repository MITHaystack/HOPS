#
# Makefile for C++ Mk4 Interface Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

pkginclude_HEADERS = include/MHOArrayMath.hh \
	include/MHOArrayOperator.hh \
	include/MHOArrayWrapper.hh \
	include/MHOAxis.hh \
	include/MHOAxisPack.hh \
	include/MHOBinaryArrayOperator.hh \
	include/MHOInPlaceUnaryArrayOperator.hh \
	include/MHOScalarContainer.hh \
	include/MHOTensorContainer.hh \
	include/MHOUnaryArrayOperator.hh \
	include/MHOVectorContainer.hh \
	include/MHOVisibilities.hh

#pkglib_LTLIBRARIES = libMHOXXX.la

check_PROGRAMS = TestArrayWrapper TestScalarContainer \
	TestVectorContainer TestTensorContainer

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

cxxflags = -I. -I$(srcdir)/include -Wall -Wextra
cxxmho = -I$(srcdir)/../Containers/include \
	-I$(srcdir)/../Utilities/include
AM_CXXFLAGS = $(cxxflags) $(cxxmho)

#libMHOXXX_la_SOURCES = src/MHOUXXX.cc
#libMHOXXX_la_LDFLAGS = -avoid-version

utilities = -L../Utilities/.libs -lMHOUtilities
mholibs = $(utilities)
TestArrayWrapper_SOURCES = test/TestArrayWrapper.cc
TestArrayWrapper_LDADD = $(mholibs)
TestScalarContainer_SOURCES = test/TestScalarContainer.cc
TestScalarContainer_LDADD = $(mholibs)
TestVectorContainer_SOURCES = test/TestVectorContainer.cc
TestVectorContainer_LDADD = $(mholibs)
TestTensorContainer_SOURCES = test/TestTensorContainer.cc
TestTensorContainer_LDADD = $(mholibs)


EXTRA_DIST =

#
# eof
#
