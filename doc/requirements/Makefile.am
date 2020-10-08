#
# documents
#
# SUBDIRS = .

# list the sources in $(srcdir)
tex_sources = reqs.tex

# things for the tarball
EXTRA_DIST = $(texdistfiles)

if WANT_DOCS
# to be somewhat surgical, this is useful:
# TEXINPUTS=... kpsewhich -show-path tex "filename"
# KPSETEXMFPATH set to : in configure pulls in the rest of TeX
TEXINPUTS=.:$(srcdir):$(srcdir)/../common@KPSETEXMFPATH@
# these must be different for the rules below
TEXSRC=reqs
TEXPDF=requirements

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

CLEANFILES = $(TEXSRC).* $(TEXPDF).pdf force

else
# nothing built, installed or cleaned
endif

#
# eof
#
