#
# Top-level Makefile
#
SUBDIRS = doc source data

ACLOCAL_AMFLAGS = -I m4

# this doesn't include cmake stuff yet
EXTRA_DIST = autogen.sh README.md

pkginclude_HEADERS = mho_config.h hops_config.h

# this is an unexpected (for automake) configure file
CLEANFILES = config.summary mhops-@VERSION@.tar.gz
CLEANFILES += hops_config.h

#
# eof
#
