#
# $Id$
#

SUBDIRS = .

AM_CPPFLAGS = -I. @HEADER_INCPATH_STR@ @CMAKE_C_FLAGS@

bin_PROGRAMS = @EXENAME@

@EXENAME@_SOURCES = @SOURCE_BASENAMES_STR@

@EXENAME@_LDADD = @AM_LIBLIST_STR@

#added manually (should this be configured?)
@EXENAME@_LDADD += -lm

#
# eof
#
