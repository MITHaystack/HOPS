#
# Makefile for HOPS3 integration tests
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

# reference copies of the HOPS3 tests are in the hops3*test directories

# john hacked these in with CMAKE ; now need to merge the two schemes
# -rwxrwxr-x. 1 gbc gbc 1088 Jul 29 15:45 chk_ff_3571.sh
# -rwxrwxr-x. 1 gbc gbc 1054 Jul 29 15:45 chk_ff_3413.sh
# -rwxrwxr-x. 1 gbc gbc  987 Jul 29 15:45 chk_ff_3372.sh
# -rwxrwxr-x. 1 gbc gbc  891 Jul 29 15:45 chk_ff_2843.sh
# -rwxrwxr-x. 1 gbc gbc  939 Jul 29 15:45 chk_ff_2836.sh

# create new tests with, e.g:
#  /home/gbc/HOPS/hops-git/data/switches/template.sh name
# these may not be needed in TESTS_ENVIRONMENT
#	PGPLOT_FONT=@PGPLOT_FONT@ \
#	PGPLOT_RGB=@PGPLOT_RGB@

dist_noinst_SCRIPTS = chk_2836.sh chk_2843.sh chk_3372.sh

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
