#
# Makefile for the fourfit plotting library
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
	-I$(srcdir)/../../vex/include -I$(srcdir)/../ffcontrol/include \
	-I$(srcdir)/../ffcore/include -I$(srcdir)/../ffio/include \
	-I$(srcdir)/../ffmath/include -I$(srcdir)/../ffsearch/include \
	-Wall -Wextra -DHAVE_CONFIG_H

AM_CPPFLAGS += ${FFTW3_CFLAGS} ${X_CFLAGS} -I${PGPLOT_DIR}

pkginclude_HEADERS = include/meta_struct.h include/plot_data_dir.h

pkglib_LTLIBRARIES = libffplot.la

libffplot_la_LDFLAGS = -avoid-version

libffplot_la_SOURCES = src/display_fplot.c src/generate_graphs.c \
	src/generate_text.c src/grid.c src/make_plotdata.c \
	src/make_postplot.c src/output.c src/plot_complex.c \
	src/plot_graph.c src/put_char_at.c src/sprint_char_arr.c \
	src/plot_data_dir.c src/fit_vbp.c

EXTRA_DIST = 

#
# eof
#
