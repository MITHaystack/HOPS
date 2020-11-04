#
# Makefile for python bindings to C/C++
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
# This directory supports compilation of a SWIG-based interface library
# which allows python to call C/C++ functionality.  include/*.i are
# interface specifications for SWIG to read to generate .c files for
# this library.  Additional src/*.c src/*.cc files supply "glue" to
# simplify the interface.
#
# The include/numpy.i interface is obtained from github/numpy
#

pkginclude_HEADERS = include/wrap_msg.h

#pkglib_LTLIBRARIES = libMHOXXX.la

check_PROGRAMS = TestWrapMessage

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS)
# XFAIL_TESTS =

# special compilation arrangements for coverage tests, see below
if DO_COVER
coververbose = -DCOVERVERB=1 -DSTATIC=extern
covercompile = --coverage -fkeep-inline-functions -fkeep-static-functions
coverldflags = -L. --coverage -DSTATIC=extern
else # DO_COVER
coververbose = -DCOVERVERB=0 -DSTATIC=static
covercompile =
coverldflags = -L. -DSTATIC=static
endif # DO_COVER

defines = -DHOPS_ENABLE_DEBUG_MSG=1 $(covercompile) $(coververbose)
AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall -Wextra $(defines)
AM_CPPFLAGS = -I. -I$(srcdir)/include -Wall -Wextra $(defines)
AM_LDFLAGS = $(coverldflags)

#libMHOXXX_la_SOURCES = src/MHOUXXX.cc
#libMHOXXX_la_LDFLAGS = -avoid-version

TestWrapMessage_SOURCES = src/wrap_msg.cc test/TestWrapMessage.c
TestWrapMessage_CXXFLAGS = -I$(srcdir)/../../cpp_src/Message/include
TestWrapMessage_LDADD = -L../../cpp_src/Message/.libs -lMHOMessage

#pyhops.c: $(srcdir)/include/pyhops.i 

# coverage processing
if DO_COVER
co = -fpm # 'make coverage co= ' to get help on coverep.sh
covclean = *.gcno *.gcda *.gcov *.log coverage-*.txt
coverage-local:
	@$(top_srcdir)/m4/coverep.sh \
		$(GCOV) $(co) $(abs_srcdir) $(abs_builddir) $(top_srcdir)
else # DO_COVER
coverage-local:
	@echo coverage is disabled, configure with --enable-gcov
endif # DO_COVER


# one-time import rule
githuburl = https://raw.githubusercontent.com
numpy_master_swigs = \
	$(srcdir)/include/numpy.i \
	$(srcdir)/include/numpy-README.txt \
	$(srcdir)/include/numpy-LICENSE.txt
one-time-import: $(numpy_master_swigs)
$(srcdir)/include/numpy.i:
	wget $(githuburl)/numpy/numpy/master/tools/swig/numpy.i
	mv numpy.i $(srcdir)/include/numpy.i
$(srcdir)/include/numpy-README.txt:
	wget $(githuburl)/numpy/numpy/master/tools/swig/README
	mv README $(srcdir)/include/numpy-README.txt
$(srcdir)/include/numpy-LICENSE.txt:
	wget $(githuburl)/numpy/numpy/master/LICENSE.txt
	mv LICENSE.txt $(srcdir)/include/numpy-LICENSE.txt

clean-local:
	-rm -rf __pycache__

CLEANFILES = $(covclean)

EXTRA_DIST = $(numpy_master_swigs)

#
# eof
#
