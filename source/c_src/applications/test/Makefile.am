#
# Makefile for fourfit (and other) tests
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#

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

SUBDIRS =

TESTS_ENVIRONMENT = \
	abs_top_builddir=$(abs_top_builddir) \
	abs_top_srcdir=$(abs_top_srcdir) \
	srcdir=$(srcdir)

TESTS = chk_2836.sh

CLEANFILES = ff-2836.ps

clean-local:

#
# eof
#
