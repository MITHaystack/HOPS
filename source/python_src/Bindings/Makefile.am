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
# The SWIG machinery is written so that swig will be used to build the
# intefaces (which then can be deleted by clean) if present.
#
# The include/numpy.i interface is obtained from github/numpy
#

# module name for python
pymod = mhops
# automake equivalent ('_' illegal as first character)
ammod = SWIGmhops

# $(pymod).la: we use SWIG for '_' which is illegal as a first character
pkgpyexec_LTLIBRARIES = SWIGmhops.la
pkglib_LTLIBRARIES = libMHOwrap.la

# local machinery
pkginclude_HEADERS = include/wrap_msg.h
noinst_HEADERS = include/npary_demo.h

# bin_PROGRAMS
pkgpython_PYTHON = $(pymod).py

check_PROGRAMS = TestWrapMessage
dist_check_SCRIPTS = ensure_esso.sh test/TestSWIG$(pymod).py \
	test/TestNumpy$(pymod).py

# man pages
dist_man_MANS = TestWrapMessage.1 TestNumpymhops.py.1

TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(check_PROGRAMS) $(dist_check_SCRIPTS)
#XFAIL_TESTS = test/TestNumpy$(pymod).py

# special compilation arrangements for coverage tests, see below
if DO_COVER
coververbose = -DCOVERVERB=1 -DSTATIC=extern
#covercompile = --coverage -fkeep-inline-functions -fkeep-static-functions
covercompile = --coverage -fkeep-inline-functions @KEEP_STATIC_FUNCTIONS@
coverldflags = -L. --coverage -DSTATIC=extern
#coverldlibs = -lgcov
if COVER_HACK
coverfinal = test/atexit.c
else # COVER_HACK
coverfinal =
endif # COVER_HACK
else # DO_COVER
coververbose = -DCOVERVERB=0 -DSTATIC=static
covercompile =
coverldflags = -L. -DSTATIC=static
#coverldlibs =
coverfinal =
endif # DO_COVER

# convenience vars for readabilty
msinc = -I$(srcdir)/../../cpp_src/Message/include
mslib = -L../../cpp_src/Message/.libs
defines = -DHOPS_ENABLE_DEBUG_MSG=1 $(covercompile) $(coververbose)
preproc = -I. -I$(srcdir)/include -Wall -Wextra $(defines)

AM_CXXFLAGS = $(preproc)
AM_CPPFLAGS = $(preproc)
AM_LDFLAGS = $(coverldflags)

# as proof of principal we build the wrapper library independently
# of the module library which we only need for python.
libMHOwrap_la_SOURCES = src/wrap_msg.cc
libMHOwrap_la_CXXFLAGS = $(msinc)
libMHOwrap_la_LIBADD = $(mslib) -lMHOMessage $(coverldlibs)

# this is a direct test of the library source
#TestWrapMessage_SOURCES = src/wrap_msg.cc test/TestWrapMessage.c
#TestWrapMessage_CXXFLAGS = $(msinc)
#TestWrapMessage_LDADD = $(mslib) -lMHOMessage
# this tests the same code via the library
TestWrapMessage_SOURCES = test/TestWrapMessage.c $(coverfinal)
TestWrapMessage_CXXFLAGS = $(msinc)
TestWrapMessage_LDADD = $(mslib) -lMHOMessage -L.libs -lMHOwrap $(coverldlibs)


# SWIG machinery
swigw = -Wno-missing-field-initializers
swigw += -Wno-unused-parameter
#pyinc = -I/usr/include/python$(PYTHON_VERSION)
pyinc = @PYTHON_CFLAGS@
npinc = -I@NUMPY_INCLUDE@
swig_exec = $(SWIG) -python -py3 -I$(srcdir)/include -module $(pymod)
swig_interfaces = include/$(pymod).i
swig_interfaces += include/wrap_msg.i
swig_interfaces += include/npary_demo.i
swig_interfaces += include/npary_helpers.py
swig_products = $(pymod).c $(pymod).py
swig_testing = _$(pymod).so ensure_esso.sh
if HAVE_SWIG
swig_delete = swig.log $(swig_products) $(swig_testing)
else # HAVE_SWIG
swig_delete = swig.log $(swig_testing)
endif # HAVE_SWIG

# generates the module library $(ammod).la
#SWIGmhops_la_SOURCES = $(pymod).c src/wrap_msg.cc src/npary_demo.cc \
#	src/npary_pyvar.c $(coverfinal)
# automake v 1.13.4 cannot handle $(pymod).c here
SWIGmhops_la_SOURCES = mhops.c src/wrap_msg.cc src/npary_demo.cc \
	src/npary_pyvar.c $(coverfinal)
SWIGmhops_la_CPPFLAGS = $(coververbose) \
	$(npinc) $(pyinc) $(msinc) $(AM_CPPFLAGS) $(swigw)
SWIGmhops_la_CXXFLAGS = $(coververbose) \
	$(npinc) $(pyinc) $(msinc) $(AM_CXXFLAGS) $(swigw)
SWIGmhops_la_LDFLAGS = -avoid-version -module $(npinc) $(coverldflags)
SWIGmhops_la_LIBADD = $(mslib) -lMHOMessage $(coverldlibs)

# generate the SWIG C interface and Python bindings
# one $(swig_exec) per interface specification produces .c and .py
# since the $(swig_products) built will be captured in the distribution
# we create an empty logfile to preserve the dependencies
if HAVE_SWIG
swig.log:
	@rm -f swig.log
	$(swig_exec) -o $(pymod).c $(srcdir)/include/$(pymod).i 2>&1 |\
		tee -a swig.log
$(pymod).c: $(swig_interfaces) swig.log
$(pymod).py: $(swig_interfaces) swig.log
else # HAVE_SWIG
swig.log:
	touch swig.log
endif # HAVE_SWIG

# this makes _$(pymod).so exist locally (only for testing)
ensure_esso.sh: _$(pymod).so
	@echo '[ ! -s swig.log ]' >> $@ ; chmod +x $@
_$(pymod).so: $(pkgpyexec_LTLIBRARIES)
	@[ -f _$(pymod).so -a -f .libs/SWIG$(pymod).so ] || \
		ln -s .libs/SWIG$(pymod).so _$(pymod).so

# move the installed shared object to the name Python wants
#	-rm -f $(pkgpyexecdir)/_$(pymod).so
#	ln -fs SWIG$(pymod).so $(pkgpyexecdir)/_$(pymod).so
ouch = $(DESTDIR)/$(pkgpyexecdir)
install-exec-hook:
	[ -w $(ouch) ] || chmod +w $(ouch)
	rm -f $(ouch)/$(ammod).a $(ouch)/$(ammod).la
	mv $(ouch)/$(ammod).so $(ouch)/_$(pymod).so
uninstall-hook:
	[ -w $(ouch) ] || chmod +w $(ouch)
	rm -f $(ouch)/_$(pymod).so

# man page example -- note that in this case TestWrapMessage
# is a check function so it needs to be built prior to an install
# which is why it is listed as a dependency here.
if DO_MANPAGES
TestWrapMessage.1: $(srcdir)/test/TestWrapMessage.c TestWrapMessage
	@$(top_srcdir)/m4/help2man.sh $(HELP2MAN) $@ ./TestWrapMessage $<
TestNumpymhops.py.1: $(srcdir)/test/TestNumpymhops.py _$(pymod).so
	@$(top_srcdir)/m4/help2man.sh $(HELP2MAN) $@ $< $<
else # DO_MANPAGES
TestWrapMessage.1: $(srcdir)/doc/TestWrapMessage.1
	-[ -f $< ] && cp -p $< $@
TestNumpymhops.py.1: $(srcdir)/doc/TestNumpymhops.py.1
	-[ -f $< ] && cp -p $< $@
endif # DO_MANPAGES

# verify the installation
installcheck-local:
	[ -f $(man1dir)/TestWrapMessage.1 ]

# coverage processing
if DO_COVER
#co = -fmp # 'make coverage co= ' to get help on coverep.sh
co = -fp # 'make coverage co= ' to get help on coverep.sh
covclean = *.gcno */*.gcno *.gcda */*.gcda *.gcov */*.gcov *.log
covclean += coverage-*.txt
coverage-local:
	@GCOV_SIMPLE=$(GCOV_SIMPLE) $(top_srcdir)/m4/coverep.sh \
		$(GCOV) $(co) $(abs_srcdir) $(abs_builddir) $(top_srcdir)
else # DO_COVER
coverage-local:
	@echo coverage is disabled, configure it with --enable-gcov
endif # DO_COVER


# one-time import rule to import numpy.i w/README&LICENSE
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
CLEANFILES = $(covclean) $(swig_delete) $(dist_man_MANS)

EXTRA_DIST = $(numpy_master_swigs) $(swig_interfaces)

#
# eof
#
