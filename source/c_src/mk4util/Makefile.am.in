#
# $Id$
#

SUBDIRS = .

AM_CPPFLAGS = -I. -I$(srcdir)/@INC_RELPATH@ @CMAKE_C_FLAGS@

pkginclude_HEADERS = @HEADER_BASENAMES_STR@

pkglib_LTLIBRARIES = lib@LIBNAME@.la

lib@LIBNAME@_la_SOURCES = @SOURCE_BASENAMES_STR@

lib@LIBNAME@_la_LDFLAGS = -avoid-version

#
# eof
#
