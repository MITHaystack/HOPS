#
# Makefile for C++ Utilities Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall -Wextra

pkginclude_HEADERS = include/MHODateTime.hh \
	include/MHOInterval.hh \
	include/MHOIntervalLabel.hh \
	include/MHOIntervalLabelTree.hh \
	include/MHOMeta.hh \
	include/MHOMultiTypeMap.hh \
	include/MHONamed.hh \
	include/MHOTimer.hh \
	include/MHOTimeStampConverter.hh \
	include/MHOTokenizer.hh \
	include/MHOUnits.hh

pkglib_LTLIBRARIES = libMHOUtilities.la

check_PROGRAMS = TestIntervalLabelTree TestMultiTypeMap TestTimeStampConverter

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

libMHOUtilities_la_SOURCES = src/MHODateTime.cc \
	src/MHOIntervalLabel.cc \
	src/MHOIntervalLabelTree.cc \
	src/MHONamed.cc \
	src/MHOTimer.cc \
	src/MHOTimeStampConverter.cc
libMHOUtilities_la_LDFLAGS = -avoid-version

#v4
thislib = -L.libs -lMHOUtilities

TestIntervalLabelTree_SOURCES = test/TestIntervalLabelTree.cc
TestIntervalLabelTree_LDADD = $(thislib)
TestMultiTypeMap_SOURCES = test/TestMultiTypeMap.cc
TestMultiTypeMap_LDADD = $(thislib)
TestTimeStampConverter_SOURCES = test/TestTimeStampConverter.cc
TestTimeStampConverter_LDADD = $(thislib)

EXTRA_DIST =

#
# eof
#
