#
# Makefile for the original chops/source/c_src/mk4util
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I../../.. -Wall -Wextra -DHAVE_CONFIG_H

pkginclude_HEADERS = include/account.h include/fileset.h include/fstruct.h \
	include/general.h include/mk4_sizes.h include/mk4_typedefs.h \
	include/mk4_util.h include/adler32_checksum.h

pkglib_LTLIBRARIES = libmk4util.la

libmk4util_la_SOURCES = src/account.c src/check_name.c src/clear_date.c \
	src/clear_fstruct.c src/confirm.c src/datec_to_datef.c \
	src/datef_to_datec.c src/day_of_datef.c src/environment.c \
	src/extract_filenames.c src/fileset.c src/get_abs_path.c \
	src/get_filelist.c src/hptoie.c src/hptoie4.c src/hptoie8.c \
	src/int_to_time.c src/ismk4.c src/minmax.c src/msg.c \
	src/report_times.c src/root_belong.c src/root_id.c \
	src/sexigesimal2hrdeg.c src/sort_names.c src/swap.c \
	src/syntax.c src/time_to_double.c src/time_to_int.c \
	src/adler32_checksum.c src/mk4stub.c

libmk4util_la_LDFLAGS = -avoid-version

EXTRA_DIST =

#
# eof
#
