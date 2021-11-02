#
# Makefile for the fourfit math library
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../dfio/include $(MHO_CPPFLAGS)

pkginclude_HEADERS = include/ffmath.h

pkglib_LTLIBRARIES = libffmath.la

libffmath_la_SOURCES = src/ap_mean.c src/bcd_to_2int.c \
	src/interp555.c src/linterp.c src/max555.c \
	src/parabola.c src/minvert.c

libffmath_la_LDFLAGS = -avoid-version

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS =

EXTRA_DIST = CMakeLists.txt

#
# eof
#
