#
# makefile for Tasking machinery
# (c) Massachusetts Institute of Technology, 2021
# The contents of the package Copyright statement apply here.
#
# This makefile runs the tasking machinery to generate a tasking document
# which provides some materials for the SDP or the specifications doc.
#

# $(TEXPDF).pdf is built from $(TEXSRC).tex
TEXPDF=HOPS4-tasking
TEXSRC=tasks

# how generated task files get named; change this to capture
# historical copies at various times with 'make save'
# mho=alt gets you the alt version
mho=mho

# everything from $(srcdir) included from $(TEXSRC).tex
# needs to be listed here with a $(srcdir)/ path prefix
appsamples = apptasks/APP_task.txt apptasks/APP_task_inputs.txt \
	apptasks/APP_task_bubbles.pl apptasks/APP_task_config.pl
# mhoinputs shall be stable for a bit
mhoinputs = mhotasks/MHO_task.txt mhotasks/MHO_task_inputs.txt \
	mhotasks/MHO_task_bubbles.pl mhotasks/MHO_task_config.pl
# a hacking around version
altinputs = alttasks/MHO_task_inputs.txt \
	alttasks/MHO_task_0000.txt alttasks/MHO_task_ZZZZ.txt \
	alttasks/MHO_task_0001.txt alttasks/MHO_task_0010.txt \
	alttasks/MHO_task_0100.txt alttasks/MHO_task_0200.txt \
	alttasks/MHO_task_0300.txt alttasks/MHO_task_0400.txt \
	alttasks/MHO_task_0500.txt \
	alttasks/MHO_task_bubbles.pl alttasks/MHO_task_config.pl

# the perl scripts that run the task machinery
taskingscripts = taskscripts/sw_tasks.pl taskscripts/sw_task_graph.pl \
	taskscripts/sw_task_mjd.pl taskscripts/sw_task_parser.pl \
	taskscripts/sw_task_timeline.pl taskscripts/sw_task_wbs.pl

EXTRA_DIST = $(srcdir)/$(TEXSRC).tex \
	$(appsamples) $(mhoinputs) $(altinputs) $(taskingscripts)

if HAVE_PERL
# this pulls everything in from source dir if we are in build dir
# note that the dependency is on all possible inputs, but the rsync
# will only pull in the group selected by $(mho)
rsopts = -v --exclude '.*.swp'
sync: $(taskingscripts) $(mhoinputs) $(altinputs)
	[ -f Makefile.am ] || \
	rsync -a $(rsopts) $(srcdir)/taskscripts/ $(srcdir)/$(mho)tasks/ .
	touch sync

# this builds everything
DOTDEV = pdf
mhoarg = -d ${DOTDEV} \
	 -i MHO_task.txt \
	 -b MHO_task_bubbles.pl \
	 -c MHO_task_config.pl
# additional args
mhoopt = 
fruit:	Makefile sync
	-[ $(mho) = alt ] && cat MHO_task_[0Z]*.txt > MHO_task.txt
	./sw_tasks.pl -v $(mhoarg) -o fruit/$(mho) -r all -g ALL $(mhoopt)
	[ -d fruit ]

# this installs the 'new' task file to the srcdir
new:
	cp -p fruit/$(mho)*.new $(srcdir)/$(mho)tasks

save: fruit
	tar zcf $(srcdir)/$(mho)tasks/save-$(mho)-`date +%Y%m%d`.tgz \
		fruit/$(mho)
	ls -l $(srcdir)/$(mho)tasks/save-$(mho)-*.tgz

else
# no task things can be generated
fruit:
	mkdir fruit
endif

tasks:
	rm -rf fruit
	$(MAKE) fruit

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
