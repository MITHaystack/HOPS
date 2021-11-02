#
# Makefile for C application fringex
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include -I$(srcdir)/../../dfio/include \
	-I$(srcdir)/../../vex/include \
	-I$(srcdir)/../../afio/include $(MHO_CPPFLAGS)

noinst_HEADERS = include/fringex.h
# pkginclude_HEADERS ?

bin_PROGRAMS = fringex

fringex_c = src/accum_segs.c src/calc_seg.c src/clear_fxp.c \
	src/clear_loops.c src/determine_nsegs.c src/filelist.c \
	src/fill_aline.c src/init_fxp.c src/model.c src/parse_cmdline.c \
	src/parse_dflag.c src/parse_iflag.c src/read_binaries.c \
	src/realloc_segs.c src/set_loops.c src/set_reffreq.c

fringex_CPPFLAGS = -DBIGGER=1 $(AM_CPPFLAGS)
fringex_SOURCES = fringex.c $(fringex_c)
fringex_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
    ../../afio/libafio.la ../../vex/libvex.la ../../msg/libmsg.la \
	$(CPGPLOT_LIBS)

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = fringex

#
# eof
#
