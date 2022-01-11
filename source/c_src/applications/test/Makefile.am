#
# Makefile for HOPS3 integration tests
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

# reference copies of the HOPS3 tests are in the hops3*test directories
# these will eventually be obliterated....

# groupings of tests in HOPS3 devel
# shortlist = chk_ff_2843.sh chk_baselines.sh chk_alist.sh chk_adump.sh
# shortlist += chk_aedit.sh chk_fringex.sh chk_fourmer.sh chk_hdlinks.sh
# devellist = chk_ff_2836.sh chk_ff_2843.sh chk_ff_3372.sh chk_ff_3413.sh
# devellist += chk_ff_3756.sh chk_baselines.sh chk_alist.sh chk_adump.sh
# devellist += chk_aedit.sh chk_fringex.sh chk_average.sh chk_avefix.sh
# devellist += chk_search.sh chk_fourmer.sh chk_frmrsrch.sh
# devellist += chk_hdlinks.sh chk_min_weight.sh chk_passband.sh
# devellist += chk_notches.sh chk_notches_xf.sh chk_ff_3772pb.sh
# hsearch = chk_flagging.sh chk_ff_dump.sh chk_ps2pdf.sh chk_ff_ps2pdf.sh
# cofit = chk_cofit.sh

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
