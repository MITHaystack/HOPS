#
# Makefile for C++ Messaging Class
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

defines = -DHOPS_ENABLE_DEBUG_MSG=0
AM_CXXFLAGS = -I$(srcdir)/include $(MHO_CXXFLAGS) $(defines)

pkginclude_HEADERS = include/MHO_Message.hh
noinst_HEADERS = include/MHO_TestAssertions.hh

pkglib_LTLIBRARIES = libMHO_Message.la

libMHO_Message_la_SOURCES = src/MHO_Message.cc
libMHO_Message_la_LDFLAGS = -avoid-version

TESTS = TestMessenger
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

check_PROGRAMS = $(TESTS)

thislib = -L.libs -lMHO_Message
TestMessenger_SOURCES = test/TestMessenger.cc
TestMessenger_LDADD = $(thislib)

EXTRA_DIST = CMakeLists.txt test/CMakeLists.txt

#
# eof
#
