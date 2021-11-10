#
# Makefile for fourfit control library
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../ffcore/include $(MHO_CPPFLAGS)

pkginclude_HEADERS = include/control.h include/parser.h include/ffcontrol.h

pkglib_LTLIBRARIES = libffcontrol.la

libffcontrol_la_SOURCES = src/copy_cblock_parts.c src/create_fsm.c \
	src/criteria_match.c src/default_cblock.c src/get_bfstring.c \
	src/init_tokens.c src/lex.c src/nullify_cblock.c \
	src/parse_control_file.c src/parser.c src/read_control_file.c \
	src/skip_data.c src/skip_index.c

libffcontrol_la_LDFLAGS = -avoid-version

# TESTS =
# TESTS_ENVIRONMENT = srcdir=$(srcdir)
# TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
# TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

# check_PROGRAMS = $(TESTS)

EXTRA_DIST = CMakeLists.txt

#
# eof
#
