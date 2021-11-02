#
# Makefile for C++ Messaging Class
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

pkginclude_HEADERS = include/MHO_Message.hh
noinst_HEADERS = include/MHO_TestAssertions.hh

pkglib_LTLIBRARIES = libMHO_Message.la

check_PROGRAMS = TestMessenger

defines = -DHOPS_ENABLE_DEBUG_MSG=1
AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall -Wextra $(defines)

libMHO_Message_la_SOURCES = src/MHO_Message.cc
libMHO_Message_la_LDFLAGS = -avoid-version

TestMessenger_SOURCES = test/TestMessenger.cc
TestMessenger_LDADD = -L.libs -lMHO_Message

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

EXTRA_DIST = CMakeLists.txt test/CMakeLists.txt

#
# eof
#
