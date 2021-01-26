#
# Makefile for C++ Messaging Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

pkginclude_HEADERS = include/MHO_BitReversalPermutation.hh \
	include/MHO_FastFourierTransformUtilities.hh

pkglib_LTLIBRARIES = libMHO_Math.la

check_PROGRAMS = TestFastFourierTransform

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

cxxflags = -I. -I$(srcdir)/include -Wall -Wextra
cxxmho = -I$(srcdir)/../Message/include \
	-I$(srcdir)/../Containers/include
AM_CXXFLAGS = $(cxxflags) $(cxxmho)

libMHO_Math_la_SOURCES = src/MHO_BitReversalPermutation.cc \
	src/MHO_FastFourierTransformUtilities.cc

libMHO_Math_la_LDFLAGS = -avoid-version

# v4
thislib = -L.libs -lMHO_Math
message = -L../Message/.libs -lMHO_Message
mholibs = $(message)

TestFastFourierTransform_SOURCES = test/TestFastFourierTransform.cc
TestFastFourierTransform_LDADD = $(thislib) $(mholibs)

EXTRA_DIST =

#
# eof
#
