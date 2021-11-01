#
# Makefile for C applications
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
    -I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
    -Wall -Wextra -DHAVE_CONFIG_H

bin_PROGRAMS = adump

adump_SOURCES = adump.c \
	src/parse_bfields.c src/parse_cmdline.c src/parse_tfields.c \
	src/strip_bline.c src/strip_tline.c src/write_header.c

adump_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
	../../msg/libmsg.la

# FIXME: check and fix
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = adump

#
# eof
#
