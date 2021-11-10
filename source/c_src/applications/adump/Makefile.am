#
# Makefile for C application adump
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
    -I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
    $(MHO_CPPFLAGS)

#noinst_HEADERS ?
pkginclude_HEADERS = include/adump.h

bin_PROGRAMS = adump

adump_SOURCES = adump.c \
	src/parse_bfields.c src/parse_cmdline.c src/parse_tfields.c \
	src/strip_bline.c src/strip_tline.c src/write_header.c

adump_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
	../../msg/libmsg.la

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = adump

#
# eof
#
