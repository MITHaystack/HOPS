#
# Makefile for python modules and scripts
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
# the parent Makefile will only bring us here if python is installed
#
if HAVE_SWIG
SUBDIRS = Bindings
else
SUBDIRS =
endif

# this is a simple test script that is configured from scripts/test.py.in
bin_SCRIPTS = scripts/test.py

#
# eof
#
