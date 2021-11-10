#
# Makefile for legacy C application CorAsc2
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include \
	-I$(srcdir)/../../../dfio/include \
	-I$(srcdir)/../../../mk4util/include \
	$(MHO_CPPFLAGS)

noinst_HEADERS = $(srcdir)/include/tapeDB.h
# pkginclude_HEADERS ?

bin_PROGRAMS = CorAsc2 CorPrune

CorAsc2_SOURCES = src/CorAsc2.c
CorPrune_SOURCES = src/CorPrune.c

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = CorAsc2 CorPrune

#
# eof
#
