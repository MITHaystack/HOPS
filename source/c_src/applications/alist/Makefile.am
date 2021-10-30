#
# Makefile for the alist application
# (c) Massachusetts Institute of Technology, 2020-2021
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
	-Wall -Wextra -DHAVE_CONFIG_H

bin_PROGRAMS = alist

alist_SOURCES = alist.c \
	src/open_output.c src/parse_cmdline.c src/summarize_mk4fringe.c

alist_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
	../../dfio/libdfio.la ../../msg/libmsg.la

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = alist

EXTRA_DIST = 

#
# eof
#
