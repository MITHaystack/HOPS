#
# Makefile for fourfit control
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../ffcore/include -Wall -Wextra -DHAVE_CONFIG_H

pkginclude_HEADERS = include/control.h include/parser.h

pkglib_LTLIBRARIES = libffcontrol.la

libffcontrol_la_SOURCES = src/copy_cblock_parts.c src/create_fsm.c \
	src/criteria_match.c src/default_cblock.c src/get_bfstring.c \
	src/init_tokens.c src/lex.c src/nullify_cblock.c \
	src/parse_control_file.c src/parser.c src/read_control_file.c \
	src/skip_data.c src/skip_index.c

libffcontrol_la_LDFLAGS = -avoid-version

EXTRA_DIST =

#
# eof
#
