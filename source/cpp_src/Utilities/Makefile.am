#
# Makefile for C++ Utilities Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall -Wextra

pkginclude_HEADERS = include/MHO_DateTime.hh \
	include/MHO_Interval.hh \
	include/MHO_IntervalLabel.hh \
	include/MHO_IntervalLabelTree.hh \
	include/MHO_Meta.hh \
	include/MHO_MultiTypeMap.hh \
	include/MHO_Named.hh \
	include/MHO_Timer.hh \
	include/MHO_TimeStampConverter.hh \
	include/MHO_Tokenizer.hh \
	include/MHO_Units.hh

pkglib_LTLIBRARIES = libMHO_Utilities.la

check_PROGRAMS = TestIntervalLabelTree TestMultiTypeMap TestTimeStampConverter

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

libMHO_Utilities_la_SOURCES = src/MHO_DateTime.cc \
	src/MHO_IntervalLabel.cc \
	src/MHO_IntervalLabelTree.cc \
	src/MHO_Named.cc \
	src/MHO_Timer.cc \
	src/MHO_TimeStampConverter.cc \
  src/MHO_Tokenizer.cc

libMHO_Utilities_la_LDFLAGS = -avoid-version

#v4
thislib = -L.libs -lMHO_Utilities

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
