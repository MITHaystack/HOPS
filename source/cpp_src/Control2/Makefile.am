#
# Makefile for C++ vex2json Class
# (c) Massachusetts Institute of Technology, 2021-2022
# The contents of the package Copyright statement apply here.
#

jsoninc = -I$(abs_top_srcdir)/extern/json/include
cppmho = -I$(srcdir)/../../c_src/vex/include \
	-I$(srcdir)/../../c_src/mk4util/include
cxxmho = -I$(srcdir)/../Message/include $(jsoninc)
AM_CXXFLAGS = -I$(srcdir)/include $(MHO_CXXFLAGS) $(cxxmho) $(cppmho)

pkginclude_HEADERS = \
	include/MHO_ChanStructWrapper.hh \
	include/MHO_DateStructWrapper.hh \
	include/MHO_JSONHeaderWrapper.hh \
	include/MHO_JSONOutputObject.hh \
	include/MHO_ScanStructWrapper.hh \
	include/MHO_SkyCoordStructWrapper.hh \
	include/MHO_SourceStructWrapper.hh \
	include/MHO_StationStructWrapper.hh

pkglib_LTLIBRARIES = libvex2json.la

libvex2json_la_SOURCES = \
	src/MHO_ChanStructWrapper.cc \
	src/MHO_DateStructWrapper.cc \
	src/MHO_ScanStructWrapper.cc \
	src/MHO_SkyCoordStructWrapper.cc \
	src/MHO_SourceStructWrapper.cc \
	src/MHO_StationStructWrapper.cc
libvex2json_la_LDFLAGS = -avoid-version

TESTS = TestParseVex TestJson TestJson2
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)
XFAIL_TESTS = TestParseVex TestJson2

check_PROGRAMS = $(TESTS)

thislib = -L.libs -lvex2json
#message = -L../Message/.libs -lMHO_Message
mk4util = -L../../c_src/mk4util/.libs -lmk4util
vexlib = -L../../c_src/vex/.libs -lvex
msglib = -L../../c_src/msg/.libs -lmsg
mholibs = $(mk4util) $(vexlib) $(msglib) $(thislib)
#$(message)
TestParseVex_SOURCES = test/TestJson2.cc
TestParseVex_LDADD = $(mholibs)
TestJson_SOURCES = test/TestJson.cc
TestJson_LDADD = $(mholibs)
TestJson2_SOURCES = test/TestParseVex.cc
TestJson2_LDADD = $(mholibs)

CLEANFILES =

EXTRA_DIST = CMakeLists.txt test/CMakeLists.txt

#
# eof
#
