#
# Makefile for C applications
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS             = test_new_chan_id

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
    -I$(srcdir)/../../mk4util/include -I$(srcdir)/../../dfio/include \
	-Wall -Wextra -DHAVE_CONFIG_H

noinst_HEADERS = include/fourmer.h

bin_PROGRAMS = fourmer
check_PROGRAMS = test_new_chan_id

fourmer_SOURCES = fourmer.c \
	src/append_sdata.c src/do_record_merge.c src/gen_new_chan_id.c \
	src/print_cdata_cmp.c src/scan_name_edit.c

fourmer_LDADD = ../../mk4util/libmk4util.la ../../dfio/libdfio.la \
	../../msg/libmsg.la

test_new_chan_id_SOURCES = test_new_chan_id.c src/gen_new_chan_id.c

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = fourmer

#
# eof
#
