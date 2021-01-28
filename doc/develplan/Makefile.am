#
# makefile for the development plan
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

# $(TEXPDF).pdf is built from $(TEXSRC).tex
TEXPDF=develplan
TEXSRC=dplan

# everything from $(srcdir) included from $(TEXSRC).tex
# needs to be listed here with a $(srcdir)/ path prefix
EXTRA_DIST = $(srcdir)/$(TEXSRC).tex \ 
	$(srcdir)/introduction.tex \
	$(srcdir)/designoutline.tex \
	$(srcdir)/detailedtimeline.tex \
	$(srcdir)/proposedplan.tex \
	$(srcdir)/MSRI-task-origv3-1.pdf \
	$(srcdir)/MSRI-task-origv3-2.pdf

if WANT_DOCS
# to be somewhat surgical, this is useful:
# TEXINPUTS=... kpsewhich -show-path tex "filename"
# KPSETEXMFPATH set to : in configure pulls in the rest of TeX
TEXINPUTS=.:$(srcdir):$(srcdir)/../common//@KPSETEXMFPATH@
TEXBIB=.:$(srcdir):$(srcdir)/../common//:
# can be overriden at the make command to see errors
TEXOPT=-interaction=batchmode

# install to share
doc_DATA = $(TEXPDF).pdf

all: $(TEXPDF).pdf

$(TEXPDF).pdf: $(TEXSRC).pdf
	cp -p $(TEXSRC).pdf $(TEXPDF).pdf

# TEXINPUTS allows sources to be found on the path
# insert enough rules for iteration to completion:
# pdflatex ... ; biber ... ; pdflatex ... ; pdflatex
$(TEXSRC).pdf: force
	@echo final iteration
	-TEXINPUTS=$(TEXINPUTS) pdflatex $(TEXOPT) $(TEXSRC).tex

force: $(srcdir)/Makefile.am $(EXTRA_DIST)
	@echo first iteration
	-TEXINPUTS=$(TEXINPUTS) pdflatex $(TEXOPT) $(TEXSRC).tex
	-TEXBIB=$(TEXBIB) biber $(TEXSRC) >$(TEXSRC)-biber.log && echo biber ok
	touch force

CLEANFILES = $(TEXSRC).* $(TEXPDF).pdf force *.log

else
# nothing built, installed or cleaned
endif

#
# eof
#
