#
# Makefile for HOPS3 integration tests
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

# reference copies of the HOPS3 tests are in the hops3*test directories

# create new tests with, e.g:
#  /home/gbc/HOPS/hops-git/data/switches/template.sh name
# these may not be needed in TESTS_ENVIRONMENT
#	PGPLOT_FONT=@PGPLOT_FONT@ \
#	PGPLOT_RGB=@PGPLOT_RGB@

dist_noinst_SCRIPTS = \
	chk_2491.sh \
	chk_2611.sh \
	chk_2836.sh \
	chk_2843.sh \
	chk_2849.sh \
	chk_2912.sh \
	chk_3064.sh \
	chk_3262.sh \
	chk_3365.sh \
	chk_3372.sh \
	chk_3413.sh \
	chk_3562.sh \
	chk_3571.sh \
	chk_3727.sh \
	chk_3756.sh \
	chk_3772.sh

TESTS = $(dist_noinst_SCRIPTS)
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)
XFAIL_TESTS = chk_2491.sh chk_2849.sh chk_2912.sh chk_3064.sh chk_3262.sh
XFAIL_TESTS += chk_3562.sh chk_3727.sh chk_3772.sh
# if defined, treats ERRORs into XFAILs
DISABLE_HARD_ERRORS = 1

CLEANFILES = ff-*.ps cf*

clean-local:

EXTRA_DIST = CMakeLists.txt

#
# eof
#
