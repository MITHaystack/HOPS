#
# Makefile for C++ Messaging Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

pkginclude_HEADERS = include/MHOBitReversalPermutation.hh \
	include/MHOFastFourierTransform.hh \
	include/MHOFastFourierTransformUtilities.hh \
	include/MHOMultidimensionalFastFourierTransformFFTW.hh \
	include/MHOMultidimensionalFastFourierTransform.hh

pkglib_LTLIBRARIES = libMHOMath.la

check_PROGRAMS = TestFastFourierTransform \
	TestMultidimensionalFastFourierTransform

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

cxxflags = -I. -I$(srcdir)/include -Wall -Wextra
cxxmho = -I$(srcdir)/../Message/include \
	-I$(srcdir)/../Containers/include
AM_CXXFLAGS = $(cxxflags)  $(cxxmho)

libMHOMath_la_SOURCES = src/MHOBitReversalPermutation.cc \
	src/MHOFastFourierTransform.cc \
	src/MHOFastFourierTransformUtilities.cc
libMHOMath_la_LDFLAGS = -avoid-version

# v4
thislib = -L.libs -lMHOMath
message = -L../Message/.libs -lMHOMessage
mholibs = $(message)

TestFastFourierTransform_SOURCES = test/TestFastFourierTransform.cc
TestFastFourierTransform_LDADD = $(thislib) $(mholibs)
TestMultidimensionalFastFourierTransform_SOURCES = \
	test/TestMultidimensionalFastFourierTransform.cc
TestMultidimensionalFastFourierTransform_LDADD = $(thislib) $(mholibs)


EXTRA_DIST =

#
# eof
#
