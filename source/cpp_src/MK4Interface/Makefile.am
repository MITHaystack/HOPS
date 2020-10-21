#
# Makefile for C++ Mk4 Interface Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CXXFLAGS = -I. -I$(srcdir)/include \
	-I$(srcdir)/../../c_src/mk4util/include \
	-I$(srcdir)/../../c_src/dfio/include \
	-I$(srcdir)/../../c_src/vex/include \
	-I$(srcdir)/../Utilities/include \
	-I$(srcdir)/../Containers/include \
	-Wall

pkginclude_HEADERS = include/MHOMK4CorelInterface.hh \
	include/MHOMK4FringeInterface.hh \
	include/MHOMK4Interface.hh \
	include/MHOMK4VexInterface.hh \
	include/MHOMK4Visibility.hh

pkglib_LTLIBRARIES = libMHOMK4Interface.la

libMHOMK4Interface_la_SOURCES = src/MHOMK4CorelInterface.cc \
	src/MHOMK4FringeInterface.cc \
	src/MHOMK4Interface.cc \
	src/MHOMK4VexInterface.cc

libMHOMK4Interface_la_LDFLAGS = -avoid-version

EXTRA_DIST =

#
# eof
#
