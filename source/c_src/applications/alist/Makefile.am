#
# Makefile for C application alist
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
	$(MHO_CPPFLAGS)

# noinst_HEADERS ?
# pkginclude_HEADERS = include/alist.h

bin_PROGRAMS = alist

alist_SOURCES = alist.c \
	src/open_output.c src/parse_cmdline.c src/summarize_mk4fringe.c

alist_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
	../../dfio/libdfio.la ../../msg/libmsg.la

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = alist

#
# eof
#
