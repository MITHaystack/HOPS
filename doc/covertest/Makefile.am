#
# makefile for coverage and test plan
#
# SUBDIRS = .

# list the sources in $(srcdir)
TEXPDF=coverandtest
TEXSRC=cotest
tex_sources = $(TEXSRC).tex # ...

# extra things for the tarball
EXTRA_DIST =

if WANT_DOCS
# to be somewhat surgical, this is useful:
# TEXINPUTS=... kpsewhich -show-path tex "filename"
# KPSETEXMFPATH set to : in configure pulls in the rest of TeX
TEXINPUTS=.:$(srcdir):$(srcdir)/../common@KPSETEXMFPATH@
# can be overriden at the make command
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
	rm force

force: $(srcdir)
	@echo first iteration
	-TEXINPUTS=$(TEXINPUTS) pdflatex $(TEXOPT) $(TEXSRC).tex
	touch force

CLEANFILES = $(TEXSRC).* $(TEXPDF).pdf force *.log

else
# nothing built, installed or cleaned
endif

#
# eof
#
