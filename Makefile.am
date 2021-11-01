#
# Top-level Makefile for HOP4
# (c) Massachusetts Institute of Technology, 2020-2021
# The contents of the package Copyright statement apply here.
#
SUBDIRS = doc source data

ACLOCAL_AMFLAGS = -I m4

# this doesn't include cmake stuff yet
EXTRA_DIST = autogen.sh README.md

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

#
# eof
#
