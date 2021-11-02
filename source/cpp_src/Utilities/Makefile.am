#
# Makefile for C++ Utilities Class
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

cxxmho = -I$(srcdir)/../Message/include
AM_CXXFLAGS = -I$(srcdir)/include $(MHO_CXXFLAGS) $(cxxmho)

pkginclude_HEADERS = \
	include/MHO_BidirectionalIterator.hh \
	include/MHO_BidirectionalStrideIterator.hh \
	include/MHO_BinaryFileInterface.hh \
	include/MHO_BinaryFileStreamer.hh \
	include/MHO_ClassIdentity.hh \
	include/MHO_ClassIdentityMap.hh \
	include/MHO_DateTime.hh \
	include/MHO_DirectoryInterface.hh \
	include/MHO_ExtensibleElement.hh \
	include/MHO_FileKey.hh \
	include/MHO_FileStreamer.hh \
	include/MHO_Interval.hh \
	include/MHO_IntervalLabel.hh \
	include/MHO_IntervalLabelTree.hh \
	include/MHO_MD5HashGenerator.hh \
	include/MHO_Meta.hh \
	include/MHO_MultiTypeMap.hh \
	include/MHO_ObjectTags.hh \
	include/MHO_Serializable.hh \
	include/MHO_TemplateTypenameDeduction.hh \
	include/MHO_Timer.hh \
	include/MHO_TimeStampConverter.hh \
	include/MHO_Tokenizer.hh \
	include/MHO_Units.hh \
	include/MHO_UUIDGenerator.hh \
	include/MHO_UUID.hh \
	include/picohash.h

pkglib_LTLIBRARIES = libMHO_Utilities.la

libMHO_Utilities_la_SOURCES = \
	src/MHO_BinaryFileStreamer.cc \
	src/MHO_DateTime.cc \
	src/MHO_DirectoryInterface.cc \
	src/MHO_IntervalLabel.cc \
	src/MHO_IntervalLabelTree.cc \
	src/MHO_ObjectTags.cc \
	src/MHO_Timer.cc \
	src/MHO_TimeStampConverter.cc \
	src/MHO_Tokenizer.cc \
	src/MHO_UUIDGenerator.cc
libMHO_Utilities_la_LDFLAGS = -avoid-version

TESTS = TestMultiTypeMap TestIntervalLabelTree TestTimeStampConverter \
	TestUUIDGenerator TestObjectTags TestDirectoryInterface
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS_ENVIRONMENT += abs_top_srcdir=$(abs_top_srcdir)
TESTS_ENVIRONMENT += abs_top_builddir=$(abs_top_builddir)

check_PROGRAMS = $(TESTS)

thislib = -L.libs -lMHO_Utilities
message = -L../Message/.libs -lMHO_Message
TestMultiTypeMap_SOURCES = test/TestMultiTypeMap.cc
TestMultiTypeMap_LDADD = $(thislib) $(message)
TestIntervalLabelTree_SOURCES = test/TestIntervalLabelTree.cc
TestIntervalLabelTree_LDADD = $(thislib) $(message)
TestTimeStampConverter_SOURCES = test/TestTimeStampConverter.cc
TestTimeStampConverter_LDADD = $(thislib) $(message)
TestUUIDGenerator_SOURCES = test/TestUUIDGenerator.cc
TestUUIDGenerator_LDADD = $(thislib) $(message)
TestObjectTags_SOURCES = test/TestObjectTags.cc
TestObjectTags_LDADD = $(thislib) $(message)
TestDirectoryInterface_SOURCES = test/TestDirectoryInterface.cc
TestDirectoryInterface_LDADD = $(thislib) $(message)

CLEANFILES = test-label-tree.bin test-tags.bin

EXTRA_DIST = CMakeLists.txt test/CMakeLists.txt

#
# eof
#
