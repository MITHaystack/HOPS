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

# module name
pymod = mhops
# the library name should be related to $(pymod)
pkgpyexec_LTLIBRARIES = SWIGmhops.la

# local machinery
pkginclude_HEADERS = include/wrap_msg.h
noinst_HEADERS =

# bin_PROGRAMS
pkgpython_PYTHON = $(pymod).py

check_PROGRAMS = TestWrapMessage
dist_check_SCRIPTS = ensure_esso.sh test/TestSWIG$(pymod).py

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS) $(dist_check_SCRIPTS)
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

# convenience vars for readabilty
msinc = -I$(srcdir)/../../cpp_src/Message/include
mslib = -L../../cpp_src/Message/.libs
defines = -DHOPS_ENABLE_DEBUG_MSG=1 $(covercompile) $(coververbose)

AM_CXXFLAGS = -I. -I$(srcdir)/include -Wall -Wextra $(defines)
AM_CPPFLAGS = -I. -I$(srcdir)/include -Wall -Wextra $(defines)
AM_LDFLAGS = $(coverldflags)

TestWrapMessage_SOURCES = src/wrap_msg.cc test/TestWrapMessage.c
TestWrapMessage_CXXFLAGS = $(msinc)
TestWrapMessage_LDADD = $(mslib) -lMHOMessage

# SWIG machinery
swigw = -Wno-missing-field-initializers
swiggage = $(pymod).c $(pymod).py _$(pymod).so ensure_esso.sh
swig_interfaces = include/wrap_msg.i
pyinc = -I/usr/include/python$(PYTHON_VERSION)

# generates the module library
SWIGmhops_la_SOURCES = $(pymod).c src/wrap_msg.cc
SWIGmhops_la_CPPFLAGS = $(coververbose) \
	$(pyinc) $(msinc) $(AM_CPPFLAGS) $(swigw)
SWIGmhops_la_CXXFLAGS = $(coververbose) \
	$(pyinc) $(msinc) $(AM_CXXFLAGS) $(swigw)
SWIGmhops_la_LDFLAGS = -avoid-version -module $(coverldflags)
SWIGmhops_la_LIBADD = $(mslib) -lMHOMessage

# this forces C++ linking on the module; note dummy.cxx need not exist.
# nodist_EXTRA_SWIGmhops_la_SOURCES = dummy.cxx

# generate the SWIG C interface and Python bindings
# at the moment, there is only one source for the module
# this rule changes with multiple sources
$(pymod).c: include/wrap_msg.i
	$(SWIG) -python -py3 -I$(srcdir)/include -o $@ $< 2>&1 | tee swig.log
$(pymod).py: $(pymod).c

# two rules to make the SWIG library available locally and as installed
# _$(pymod).so needs to exist locally only for testing
ensure_esso.sh: _$(pymod).so
	echo true > $@ ; chmod +x $@
_$(pymod).so: $(pkgpyexec_LTLIBRARIES)
	@[ -f _$(pymod).so -a -f .libs/SWIG$(pymod).so ] || \
		ln -s .libs/SWIG$(pymod).so _$(pymod).so
install-exec-hook:
	chmod +w $(pkgpyexecdir)
	-rm -f $(pkgpyexecdir)/_$(pymod).so
	 ln -fs $(pkgpyexecdir)/SWIG$(pymod).so _$(pymod).so

# coverage processing
if DO_COVER
co = -fpm # 'make coverage co= ' to get help on coverep.sh
covclean = *.gcno */*.gcno *.gcda */*.gcda *.gcov */*.gcov *.log coverage-*.txt
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


# cleanup and distribution
CLEANFILES = $(covclean) $(swiggage)

EXTRA_DIST = $(numpy_master_swigs) $(swig_interfaces)

#
# eof
#
