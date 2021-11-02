#
# Makefile for C application fplot
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include -I$(srcdir)/../../dfio/include \
	$(MHO_CPPFLAGS) -DPS2PDF=\"@PS2PDF@\"

noinst_HEADERS = include/fplot.h
# pkginclude_HEADERS ?

bin_PROGRAMS = fplot

fplot_SOURCES = fplot.c src/parse_cmdline.c

fplot_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
    ../../msg/libmsg.la

# chk_ps2pdf.sh
# ps2pdf=@PS2PDF@
TESTS =
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = fplot

#
# eof
#
