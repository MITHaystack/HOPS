#
# Makefile for the original chops/source/c_src/mk4util
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
	-I$(srcdir)/../../vex/include -I$(srcdir)/../ffcontrol/include \
	-I$(srcdir)/../ffmath/include -Wall -Wextra -DHAVE_CONFIG_H

pkginclude_HEADERS = include/filter.h include/fourfit_signal_handler.h \
	include/freqlist.h include/param_struct.h include/pass_struct.h \
	include/plot_struct.h include/refringe.h include/statistics.h \
	include/write_lock_mechanism.h include/ff_misc_if.h

pkglib_LTLIBRARIES = libffcore.la

libffcore_la_SOURCES = src/check_rflist.c src/clear_freq_corel.c \
    src/clear_pass.c src/create_lockfile.c src/diff_file_phase.c \
    src/fill_param.c src/fourfit_signal_handler.c src/generate_cblock.c \
    src/get_corel_data.c src/make_flist.c src/make_passes.c \
    src/pcal_interp.c src/read_sdata.c src/refringe_list.c \
    src/set_defaults.c src/set_pointers.c src/stcount_interp.c \
    src/time_range.c src/update.c src/vrot.c src/wait_for_write_lock.c

libffcore_la_LDFLAGS = -avoid-version

EXTRA_DIST = 

#
# eof
#
