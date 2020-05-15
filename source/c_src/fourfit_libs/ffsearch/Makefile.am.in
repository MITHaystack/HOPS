#
# $Id$
#

SUBDIRS = .

if HAVE_FFTW

AM_CPPFLAGS = -I. @HEADER_INCPATH_STR@ @CMAKE_C_FLAGS@

include_HEADERS = @HEADER_BASENAMES_STR@

lib_LTLIBRARIES = lib@LIBNAME@.la

lib@LIBNAME@_la_SOURCES = @SOURCE_BASENAMES_STR@

lib@LIBNAME@_la_LIBADD = @AM_LIBLIST_STR@

lib@LIBNAME@_la_LIBADD += ${FFTW3_LIBS}

lib@LIBNAME@_la_LDFLAGS = -avoid-version

else !HAVE_FFTW

lib@LIBNAME@_la_SOURCES = you_lose.c

endif !HAVE_FFTW

you_lose.c: Makefile.am
	echo 'main(){return(' > $@
	echo 'puts("One or more of the required packages:")+' >> $@
	echo 'puts("PNG, X11, PGPLOT or FFTW3 is missing.")+' >> $@
	echo 'puts("Check your installation are recompile.")' >> $@
	echo ');}' >> $@

CLEANFILES = you_lose.c

#
# eof
#
