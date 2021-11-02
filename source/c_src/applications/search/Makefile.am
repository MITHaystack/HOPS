#
# Makefile for C application search
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../afio/include $(MHO_CPPFLAGS)

noinst_HEADERS = include/search.h
# pkginclude_HEADERS ?

bin_PROGRAMS = search

search_c = src/clear_srchdata.c src/fill_grids.c src/fit_peaks.c \
	src/grid_filler.c src/parabola.c src/parse_cmdline.c src/plot_srchdata.c \
	src/read_data.c src/sort_data.c src/sorter.c src/write_srchdata.c

# revised code
search_CPPFLAGS = -DBIGGER=1 $(AM_CPPFLAGS)
search_SOURCES = search.c $(search_c)
search_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
	../../afio/libafio.la ../../msg/libmsg.la $(CPGPLOT_LIBS)

# original code
soirch_CPPFLAGS = -DBIGGER=0 $(AM_CPPFLAGS)
soirch_SOURCES = search.c $(search_c)
soirch_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
	../../afio/libafio.la ../../msg/libmsg.la $(CPGPLOT_LIBS)

# chk_ps2pdf.sh
# ps2pdf=@PS2PDF@
TESTS             = soirch
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

check_PROGRAMS = soirch

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = search

#
# eof
#
