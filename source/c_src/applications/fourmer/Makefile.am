#
# Makefile for C application fourmer
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
    -I$(srcdir)/../../mk4util/include -I$(srcdir)/../../dfio/include \
	$(MHO_CPPFLAGS)

noinst_HEADERS = include/fourmer.h
# pkginclude_HEADERS ?

bin_PROGRAMS = fourmer

fourmer_SOURCES = fourmer.c \
	src/append_sdata.c src/do_record_merge.c src/gen_new_chan_id.c \
	src/print_cdata_cmp.c src/scan_name_edit.c

fourmer_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
	../../msg/libmsg.la

test_new_chan_id_SOURCES = test_new_chan_id.c src/gen_new_chan_id.c

TESTS = test_new_chan_id
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = fourmer

#
# eof
#
