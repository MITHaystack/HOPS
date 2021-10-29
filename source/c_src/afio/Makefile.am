#
# Makefile for the A list library
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../msg/include \
	-I$(srcdir)/../mk4util/include -Wall -Wextra -DHAVE_CONFIG_H

pkginclude_HEADERS = include/adata.h include/afile_structure.h \
	include/mk4_afio.h

pkglib_LTLIBRARIES = libafio.la

libafio_la_SOURCES = src/afile_header.c src/aline_id.c src/check_sizes.c \
	src/clear_afile_structure.c src/clear_csumm.c src/clear_fsumm.c \
	src/clear_rsumm.c src/clear_tsumm.c src/corelname.c src/fringename.c \
	src/get_unique_name.c src/parse_csumm.c src/parse_fsumm.c \
	src/parse_rsumm.c src/parse_tsumm.c src/read_afile.c src/rootname.c \
	src/write_csumm.c src/write_fsumm.c src/write_rsumm.c src/write_tsumm.c

libafio_la_LDFLAGS = -avoid-version

EXTRA_DIST = 

#
# eof
#
