#
# Makefile for C++ Mk4 Interface Class
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

AM_CXXFLAGS = -I. -I$(srcdir)/include \
	-I$(srcdir)/../../c_src/mk4util/include \
	-I$(srcdir)/../../c_src/dfio/include \
	-I$(srcdir)/../../c_src/vex/include \
	-I$(srcdir)/../Containers/include \
	-I$(srcdir)/../Utilities/include \
	-I$(srcdir)/../MK4Interface/include \
	-I$(srcdir)/../Messaging/include \
	-Wall -Wextra

check_PROGRAMS = \
	TestMK4CorelInterface TestMK4Import \
	TestMK4Import2 TestMK4Interface TestMK4VexInterface \
	TestMultiTypeMap TestScalarContainer \
	TestVectorContainer

# TestMessenger TestArrayWrapper TestIntervalLabelTree TestTimeStampConverter

# ls -1 *.cc | awk -F. '{print $1 "_SOURCES = " $1 "." $2}'
#TestArrayWrapper_SOURCES = TestArrayWrapper.cc
#TestIntervalLabelTree_SOURCES = TestIntervalLabelTree.cc
#TestMessenger_SOURCES = TestMessenger.cc
TestMK4CorelInterface_SOURCES = TestMK4CorelInterface.cc
TestMK4Import2_SOURCES = TestMK4Import2.cc
TestMK4Import_SOURCES = TestMK4Import.cc
TestMK4Interface_SOURCES = TestMK4Interface.cc
TestMK4VexInterface_SOURCES = TestMK4VexInterface.cc
TestMultiTypeMap_SOURCES = TestMultiTypeMap.cc
TestScalarContainer_SOURCES = TestScalarContainer.cc
#TestTensorContainer_SOURCES = TestTensorContainer.cc
#TestTimeStampConverter_SOURCES = TestTimeStampConverter.cc
TestVectorContainer_SOURCES = TestVectorContainer.cc

# use a general rule here
LDADD = \
	-L../Messaging/.libs -lMHOMessage \
	-L../Utilities/.libs -lMHOUtilities \
	-L../MK4Interface/.libs -lMHOMK4Interface \
	-L../../c_src/dfio/.libs -ldfio \
	-L../../c_src/vex/.libs -lvex \
	-L../../c_src/mk4util/.libs -lmk4util

#	-L../../c_src/afio/.libs -lafio 

EXTRA_DIST =

#
# eof
#
