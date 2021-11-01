#
# Makefile for C applications
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#

TESTS_ENVIRONMENT = srcdir=$(srcdir) ps2pdf=@PS2PDF@
TESTS             = soirch chk_ps2pdf.sh
XFAIL_TESTS       = soirch

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../afio/include -Wall -Wextra -DHAVE_CONFIG_H

noinst_HEADERS = include/search.h

bin_PROGRAMS = search
check_PROGRAMS = soirch

search_c = src/clear_srchdata.c src/fill_grids.c src/fit_peaks.c \
	src/grid_filler.c src/parabola.c src/parse_cmdline.c src/plot_srchdata.c \
	src/read_data.c src/sort_data.c src/sorter.c src/write_srchdata.c

# revised code
search_CPPFLAGS = -DBIGGER=1 $(AM_CPPFLAGS)
search_SOURCES = search.c $(search_c)
search_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
	../../afio/libafio.la ../../msg/libmsg.la ${CPGPLOT_LIBS}

# original code
soirch_CPPFLAGS = -DBIGGER=0 $(AM_CPPFLAGS)
soirch_SOURCES = search.c $(search_c)
soirch_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
	../../afio/libafio.la ../../msg/libmsg.la ${CPGPLOT_LIBS}

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = search

#
# eof
#
