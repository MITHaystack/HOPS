#
# Makefile for C++ Messaging Class
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

cxxmho = -I$(srcdir)/../Message/include # -I$(srcdir)/../Containers/include
AM_CXXFLAGS = -I$(srcdir)/include $(cxxmho) $(MHO_CXXFLAGS)

pkginclude_HEADERS = include/MHO_BitReversalPermutation.hh \
	include/MHO_FastFourierTransformUtilities.hh \
	include/MHO_CheckForNaN.hh

pkglib_LTLIBRARIES = libMHO_Math.la

libMHO_Math_la_SOURCES = src/MHO_BitReversalPermutation.cc
libMHO_Math_la_LDFLAGS = -avoid-version

TESTS = TestFastFourierTransform
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

check_PROGRAMS = $(TESTS)

thislib = -L.libs -lMHO_Math
message = -L../Message/.libs -lMHO_Message
TestFastFourierTransform_SOURCES = test/TestFastFourierTransform.cc
TestFastFourierTransform_LDADD = $(thislib) $(message)

EXTRA_DIST = CMakeLists.txt test/CMakeLists.txt

#
# eof
#
