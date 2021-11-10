#
# Makefile for the fourfit io library
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include -I$(srcdir)/../../afio/include \
	-I$(srcdir)/../../dfio/include -I$(srcdir)/../../vex/include \
	-I$(srcdir)/../ffcontrol/include -I$(srcdir)/../ffcore/include \
	$(MHO_CPPFLAGS)

pkginclude_HEADERS = include/ffio.h

pkglib_LTLIBRARIES = libffio.la

libffio_la_SOURCES = src/fill_200.c src/fill_201.c src/fill_202.c \
	src/fill_203.c src/fill_204.c src/fill_205.c src/fill_206.c \
	src/fill_207.c src/fill_208.c src/fill_210.c src/fill_212.c \
	src/fill_222.c src/fill_230.c src/fill_fringe_info.c

libffio_la_LDFLAGS = -avoid-version

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

#
# eof
#
