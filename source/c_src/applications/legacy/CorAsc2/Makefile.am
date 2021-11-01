#
# Makefile for legacy CorAsc2 dumper
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#

SUBDIRS = 

AM_CPPFLAGS = -I$(srcdir)/include \
	-I$(srcdir)/../../../dfio/include \
	-I$(srcdir)/../../../mk4util/include

noinst_HEADERS = $(srcdir)/include/tapeDB.h

bin_PROGRAMS = CorAsc2 CorPrune

CorAsc2_SOURCES = src/CorAsc2.c
CorPrune_SOURCES = src/CorPrune.c

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = CorAsc2 CorPrune

#
# eof
#
