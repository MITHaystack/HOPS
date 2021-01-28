#
# Makefile for C++ Messaging Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

pkginclude_HEADERS = include/MHO_Message.hh

pkglib_LTLIBRARIES = libMHO_Message.la

check_PROGRAMS = TestMessenger

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

defines = -DHOPS_ENABLE_DEBUG_MSG=1
AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall -Wextra $(defines)

libMHO_Message_la_SOURCES = src/MHO_Message.cc
libMHO_Message_la_LDFLAGS = -avoid-version

TestMessenger_SOURCES = test/TestMessenger.cc
TestMessenger_LDADD = -L.libs -lMHO_Message

EXTRA_DIST =

#
# eof
#
