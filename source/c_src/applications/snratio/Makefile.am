#
# Makefile for C applications
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I$(srcdir)/../../msg/include -I$(srcdir)/../../afio/include

bin_PROGRAMS = snratio

snratio_SOURCES = snratio.c

snratio_LDADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
	../../msg/libmsg.la

#
# eof
#
