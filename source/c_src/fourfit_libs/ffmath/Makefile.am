#
# Makefile for the fourfit math library
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../dfio/include  -Wall -Wextra -DHAVE_CONFIG_H

pkginclude_HEADERS = 

pkglib_LTLIBRARIES = libffmath.la

libffmath_la_SOURCES = src/ap_mean.c src/bcd_to_2int.c \
	src/interp555.c src/linterp.c src/max555.c \
	src/parabola.c src/minvert.c

libffmath_la_LDFLAGS = -avoid-version

EXTRA_DIST =

#
# eof
#
