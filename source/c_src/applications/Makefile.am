#
# Makefile for C applications
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

# FIXME: this conditional moves down a layer once we figure
# out what we should do if FFTW3 isn't available
if HAVE_FFTW3
  SUBDIRS = fourfit alist aedit test
else
  SUBDIRS = alist aedit
endif

#
# eof
#
