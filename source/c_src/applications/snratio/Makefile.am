#
# Makefile for C application snratio
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/../../msg/include -I$(srcdir)/../../afio/include \
	$(MHO_CPPFLAGS)

bin_PROGRAMS = snratio

snratio_SOURCES = snratio.c

snratio_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
	../../msg/libmsg.la

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = snratio

#
# eof
#
