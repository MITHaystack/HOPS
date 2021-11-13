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

dist_noinst_SCRIPTS = chk_2836.sh chk_2843.sh chk_3372.sh chk_3413.sh \
	chk_3571.sh

TESTS = $(dist_noinst_SCRIPTS)
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

CLEANFILES = ff-2836.ps ff-2843.ps

clean-local:

EXTRA_DIST = CMakeLists.txt

#
# eof
#
