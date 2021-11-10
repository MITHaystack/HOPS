#
# Makefile for C fourfit libraries
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#

if HAVE_FFTW3
  SUBDIRS = ffcontrol ffcore ffio ffmath ffsearch ffplot
else
  SUBDIRS = ffcontrol ffcore ffmath
  # ffsearch rather generally
  # ffplot because of make_plotdata.c
  # ffio because of fill_230.c
endif

EXTRA_DIST =

#
# eof
#
