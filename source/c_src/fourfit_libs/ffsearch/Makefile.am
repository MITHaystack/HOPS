#
# Makefile for the fourfit search library
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include -I$(srcdir)/../../dfio/include \
	-I$(srcdir)/../../vex/include -I$(srcdir)/../ffcontrol/include \
	-I$(srcdir)/../ffcore/include -I$(srcdir)/../ffio/include \
	-I$(srcdir)/../ffmath/include  -Wall -Wextra -DHAVE_CONFIG_H

pkginclude_HEADERS = include/adhoc_flag.h include/apply_funcs.h

pkglib_LTLIBRARIES = libffsearch.la

libffsearch_la_LDFLAGS = -avoid-version

libffsearch_la_SOURCES = src/adhoc_flag.c src/apply_filter.c \
	src/apply_notches.c src/apply_passband.c src/calc_normalization.c \
	src/calc_rms.c src/compute_model.c src/compute_qf.c \
	src/create_fname.c src/delay_rate.c src/est_pc_manual.c \
	src/freq_spacing.c src/fringe_search.c src/gate_off.c \
	src/interp.c src/ion_search.c src/norm_fx.c src/norm_xf.c \
	src/organize_data.c src/parse_cmdline.c src/pcalibrate.c \
	src/precorrect.c src/report_actions.c src/rotate_pcal.c \
	src/sampler_delays.c src/search.c src/search_windows.c \
	src/update.c src/apply_video_bp.c src/ion_covariance.c \
	src/adjust_snr.c src/apply_cmplxbp.c

libffsearch_la_LIBADD = ../../mk4util/libmk4util.la ../../afio/libafio.la \
	../../dfio/libdfio.la ../../vex/libvex.la \
	../ffcontrol/libffcontrol.la ../ffcore/libffcore.la \
	../ffio/libffio.la ../ffmath/libffmath.la

EXTRA_DIST = 

#
# eof
#
