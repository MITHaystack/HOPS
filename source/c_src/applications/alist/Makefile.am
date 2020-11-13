#
# Makefile for the alist application
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
	-Wall -Wextra -DHAVE_CONFIG_H

#FIXME: name?

bin_PROGRAMS = alist

alist_SOURCES = alist.c open_output.c parse_cmdline.c summarize_mk4fringe.c

alist_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
	../../dfio/libdfio.la

#added manually (should this be configured?)  FIXME: yes
alist_LDADD += -lm

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = alist

EXTRA_DIST = 

#
# eof
#
