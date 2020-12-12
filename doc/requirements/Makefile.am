#
# makefile for requirements document
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#

# $(TEXPDF).pdf is built from $(TEXSRC).tex
TEXPDF=HOPS4-requirements
TEXSRC=reqs

# everything from $(srcdir) included from $(TEXSRC).tex
# needs to be listed here with a $(srcdir)/ path prefix
EXTRA_DIST = $(srcdir)/$(TEXSRC).tex $(srcdir)/theintro.tex \
	$(srcdir)/thereqs.tex \
	$(srcdir)/hilo.tex $(srcdir)/obsolete.tex

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
$(TEXSRC).pdf: bibs
	@echo final iteration
	-TEXINPUTS=$(TEXINPUTS) pdflatex $(TEXOPT) $(TEXSRC).tex

# see which backend was requested and invoke the appropriate bib agent
bibs: force
	@echo biber or biblatex
	-grep -q backend=biber $(srcdir)/$(TEXSRC).tex && \
		TEXBIB=$(TEXBIB) biber $(TEXSRC) >$(TEXSRC)-bibs.log || \
		BIBINPUTS=$(TEXBIB) bibtex $(TEXSRC) >$(TEXSRC)-bibs.log
	touch bibs

# a first iteration starts the ball rolling
force: $(srcdir)/Makefile.am $(EXTRA_DIST)
	@echo first iteration
	-TEXINPUTS=$(TEXINPUTS) pdflatex $(TEXOPT) $(TEXSRC).tex
	touch force

CLEANFILES = $(TEXSRC).* $(TEXPDF).pdf force bibs *.log *-blx.bib

else
# nothing built, installed or cleaned
endif

#
# eof
#
