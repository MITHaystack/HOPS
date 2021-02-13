#
# makefile for Tasking machinery
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#
# This makefile runs the tasking machinery to generate a tasking document
# which provides some materials for, the sdp or the specifications doc.
#

# $(TEXPDF).pdf is built from $(TEXSRC).tex
TEXPDF=HOPS4-tasking
TEXSRC=tasks

# how generated task files get named; change this to capture
# historical copies at various times with 'make save'
mho=mho

# everything from $(srcdir) included from $(TEXSRC).tex
# needs to be listed here with a $(srcdir)/ path prefix
appsamples = apptasks/APP_task.txt apptasks/APP_task_inputs.txt \
	apptasks/APP_task_bubbles.pl apptasks/APP_task_config.pl
mhoinputs = mhotasks/MHO_task.txt mhotasks/MHO_task_inputs.txt \
	mhotasks/MHO_task_bubbles.pl mhotasks/MHO_task_config.pl

# the perl scripts that run the task machinery
taskingscripts = taskscripts/sw_tasks.pl taskscripts/sw_task_graph.pl \
	taskscripts/sw_task_mjd.pl taskscripts/sw_task_parser.pl \
	taskscripts/sw_task_timeline.pl taskscripts/sw_task_wbs.pl

EXTRA_DIST = $(srcdir)/$(TEXSRC).tex \
	$(appsamples) $(mhoinputs) $(taskingscripts)

if HAVE_PERL
# this pulls everything in from source dir if we are in build dir
rsopts = -v --exclude '.*.swp'
sync: $(taskingscripts) $(mhoinputs)
	[ -f Makefile.am ] || \
	rsync -a $(rsopts) $(srcdir)/taskscripts/ $(srcdir)/mhotasks/ .
	touch sync

# this builds everything
mhoarg = -i MHO_task.txt -b MHO_task_bubbles.pl -c MHO_task_config.pl
fruit:	Makefile sync
	./sw_tasks.pl -v $(mhoarg) -o fruit/$(mho) -r all -g all
	[ -d fruit ]

# this installs the 'new' task file to the srcdir
new: fruit/$(mho).new
	cp -p fruit/$(mho).new $(srcdir)/mhotasks/MHO_task.new

save: fruit
	tar zcf $(srcdir)/mhotasks/save-`date +%Y%m%d`.tgz  fruit
	ls -l $(srcdir)/mhotasks/save-*.tgz

else
# no task things can be generated
fruit:
	mkdir fruit
endif

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
#force: $(srcdir)/Makefile.am $(EXTRA_DIST)
force: fruit $(EXTRA_DIST)
	@echo first iteration
	-TEXINPUTS=$(TEXINPUTS) pdflatex $(TEXOPT) $(TEXSRC).tex
	touch force

CLEANFILES = $(TEXSRC).* $(TEXPDF).pdf force bibs *.log *-blx.bib \
	sync sw_task* MHO_task* .*.swp

clean-local:
	-rm -rf fruit

else
# nothing built, installed or cleaned
endif

#
# eof
#
