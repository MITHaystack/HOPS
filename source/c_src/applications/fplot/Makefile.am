#
# Makefile for C applications
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#

TESTS_ENVIRONMENT = srcdir=$(srcdir) ps2pdf=@PS2PDF@
TESTS             = chk_ps2pdf.sh

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include -I$(srcdir)/../../dfio/include \
	-DPS2PDF=\"@PS2PDF@\" -Wall -Wextra -DHAVE_CONFIG_H

noinst_HEADERS = include/fplot.h

bin_PROGRAMS = fplot

fplot_SOURCES = fplot.c src/parse_cmdline.c

fplot_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
    ../../msg/libmsg.la

#
# eof
#
