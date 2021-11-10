#
# Makefile for C application average
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
	$(MHO_CPPFLAGS)

# noinst_HEADERS ?
pkginclude_HEADERS = include/average.h

bin_PROGRAMS = average

average_SOURCES = average.c \
	src/assess_data.c src/baseline_avg.c src/check_source.c \
	src/check_times.c src/check_tribase.c src/get_int_time.c \
	src/index_data.c src/parse_cmdline.c src/read_data.c \
	src/sort_data.c src/sorter.c src/triangle_avg.c

average_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
    ../../msg/libmsg.la

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = average

#
# eof
#
