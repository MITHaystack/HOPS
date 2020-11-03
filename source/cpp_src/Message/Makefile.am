#
# Makefile for C++ Messaging Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

pkginclude_HEADERS = include/MHOMessage.hh

pkglib_LTLIBRARIES = libMHOMessage.la

check_PROGRAMS = TestMessenger

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

defines = -DHOPS_ENABLE_DEBUG_MSG=1
AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall -Wextra $(defines)

libMHOMessage_la_SOURCES = src/MHOMessage.cc
libMHOMessage_la_LDFLAGS = -avoid-version

TestMessenger_SOURCES = test/TestMessenger.cc
TestMessenger_LDADD = -L.libs -lMHOMessage

EXTRA_DIST =

#
# eof
#
