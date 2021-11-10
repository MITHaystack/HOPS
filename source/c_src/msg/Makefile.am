#
# Makefile for the standalone (legacy) msg library
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include $(MHO_CPPFLAGS)

pkginclude_HEADERS = include/msg.h

pkglib_LTLIBRARIES = libmsg.la

libmsg_la_SOURCES = src/msg.c

libmsg_la_LDFLAGS = -avoid-version

EXTRA_DIST = CMakeLists.txt

#
# eof
#
