#
# Makefile for C applications
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
	-Wall -Wextra -DHAVE_CONFIG_H

noinst_HEADERS = include/average.h

bin_PROGRAMS = average

average_SOURCES = average.c \
	src/assess_data.c src/baseline_avg.c src/check_source.c \
	src/check_times.c src/check_tribase.c src/get_int_time.c \
	src/index_data.c src/parse_cmdline.c src/read_data.c \
	src/sort_data.c src/sorter.c src/triangle_avg.c

average_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
    ../../msg/libmsg.la

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = alist

#
# eof
#
