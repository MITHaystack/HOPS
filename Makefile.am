#
# Top-level Makefile for HOP4
# (c) Massachusetts Institute of Technology, 2020-2021
# The contents of the package Copyright statement apply here.
#
SUBDIRS = doc source data

ACLOCAL_AMFLAGS = -I m4

# it is not clear that all of these are required.
EXTRA_DIST = autogen.sh README.md
EXTRA_DIST += m4/mho.m4 m4/coverep.sh m4/hops_svn_rev.sh m4/nightlyerr.pl
EXTRA_DIST += m4/demi-setup.sh m4/help2man.sh m4/texlive-missing.sh
EXTRA_DIST += cmake/FindFFTW.cmake cmake/FindGFORTRAN.cmake cmake/FindGSL.cmake
EXTRA_DIST += cmake/FindMPI.cmake cmake/FindOpenCL.cmake cmake/FindPGPLOT.cmake
EXTRA_DIST += cmake/FindROOT.cmake cmake/HopsDefaults.cmake
EXTRA_DIST += cmake/MacroCheckPackageLibs.cmake
EXTRA_DIST += cmake/MacroCheckPackageVersion.cmake
EXTRA_DIST += cmake/MacroParseArguments.cmake cmake/UseLATEX.cmake

# For the moment, we need to re-distribute an external source
EXTRA_DIST += extern/json/include/json.hh

pkginclude_HEADERS = mho_config.h hops_config.h

# these is an unexpected (for automake) configure file
CLEANFILES = config.summary

# this turns up with dist or distchheck
CLEANFILES += mhops-@VERSION@.tar.gz

# one probably wants to force a make for this.:wq
.PHONY: log errors
log:
	-$(MAKE) all check install installcheck > $@ 2>&1
errors: log
	$(top_srcdir)/m4/nightlyerr.pl log | tee errors

# these may be produced by autogen.sh
CLEANFILES += configure.log make.log distcheck.log log errors
CLEANFILES += $(srcdir)/configure.log $(srcdir)/make.log
CLEANFILES += $(srcdir)/distcheck.log $(srcdir)/mhops-@VERSION@.tar.gz
CLEANFILES += *.tmp

# speed this process up by elminating some checking
AM_DISTCHECK_CONFIGURE_FLAGS = --disable-docs --disable-doxy --disable-sphinx

# disable distcleancheck for the moment, the doc/tasking area
# distributes some things that are also rebuilt....
distcleancheck:
	@:

#
# eof
#
