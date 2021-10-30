#
# Makefile for the fourfit executable
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CPPFLAGS = -I. -I$(srcdir)/include \
	-I$(srcdir)/../../msg/include \
	-I$(srcdir)/../../mk4util/include \
	-I$(srcdir)/../../afio/include \
	-I$(srcdir)/../../dfio/include \
	-I$(srcdir)/../../vex/include \
	-I$(srcdir)/../../fourfit_libs/ffcontrol/include \
	-I$(srcdir)/../../fourfit_libs/ffmath/include \
	-I$(srcdir)/../../fourfit_libs/ffcore/include \
	-I$(srcdir)/../../fourfit_libs/ffio/include \
	-I$(srcdir)/../../fourfit_libs/ffsearch/include \
	-I$(srcdir)/../../fourfit_libs/ffplot/include \
	-Wall -Wextra -DHAVE_CONFIG_H -DFF_PROGNAME=\"fourfit\" \
	-DFF_VER_NO=\"@VERSION@\"

AM_CPPFLAGS += ${CPGPLOT_CFLAGS} ${FFTW3_CFLAGS}

if HAVE_FFTW3
bin_PROGRAMS = fourfit
else # HAVE_FFTW3
bin_PROGRAMS =
endif # HAVE_FFTW3

fourfit_SOURCES = fourfit.c

fourfit_LDADD = ../../fourfit_libs/ffcontrol/libffcontrol.la \
    ../../fourfit_libs/ffmath/libffmath.la \
    ../../fourfit_libs/ffcore/libffcore.la \
    ../../fourfit_libs/ffsearch/libffsearch.la \
    ../../fourfit_libs/ffplot/libffplot.la \
    ../../fourfit_libs/ffio/libffio.la \
    ../../afio/libafio.la \
    ../../dfio/libdfio.la \
    ../../vex/libvex.la \
    ../../mk4util/libmk4util.la \
    ../../msg/libmsg.la

fourfit_LDADD += ${CPGPLOT_LIBS} ${FFTW3_LIBS}

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = fourfit

#
# eof
#
