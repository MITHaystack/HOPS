#
# Makefile for C++ Mk4 Interface Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

pkginclude_HEADERS = include/MHO_ArrayMath.hh \
	include/MHO_ArrayOperator.hh \
	include/MHO_ArrayWrapper.hh \
	include/MHO_Axis.hh \
	include/MHO_AxisPack.hh \
	include/MHO_BinaryArrayOperator.hh \
	include/MHO_InPlaceUnaryArrayOperator.hh \
	include/MHO_ScalarContainer.hh \
	include/MHO_TensorContainer.hh \
	include/MHO_UnaryArrayOperator.hh \
	include/MHO_VectorContainer.hh \
	include/MHO_Visibilities.hh

#pkglib_LTLIBRARIES = libMHO_XXX.la

check_PROGRAMS = TestArrayWrapper TestScalarContainer \
	TestVectorContainer TestTensorContainer

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

cxxflags = -I. -I$(srcdir)/include -Wall -Wextra
cxxmho = -I$(srcdir)/../Containers/include \
	-I$(srcdir)/../Utilities/include
AM_CXXFLAGS = $(cxxflags) $(cxxmho)

#libMHO_XXX_la_SOURCES = src/MHO_UXXX.cc
#libMHO_XXX_la_LDFLAGS = -avoid-version

utilities = -L../Utilities/.libs -lMHO_Utilities
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
