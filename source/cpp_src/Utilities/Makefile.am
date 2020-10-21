#
# Makefile for C++ Utilities Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall

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

libMHOUtilities_la_SOURCES = src/MHODateTime.cc \
	src/MHOIntervalLabel.cc \
	src/MHOIntervalLabelTree.cc \
	src/MHONamed.cc \
	src/MHOTimer.cc \
	src/MHOTimeStampConverter.cc

libMHOUtilities_la_LDFLAGS = -avoid-version

EXTRA_DIST =

#
# eof
#
