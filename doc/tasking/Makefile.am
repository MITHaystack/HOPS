#
# makefile for Tasking machinery
# (c) Massachusetts Institute of Technology, 2021-2022
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
# mho=all gets you the all version.  As needed a copy of alltasks
# can be made with a copy for other purposes.
mho=all
MHO=MHO_task

# everything from $(srcdir) included from $(TEXSRC).tex
# needs to be listed here with a $(srcdir)/ path prefix
# This is the working collection; if you adjust the set
# then you will need to clean the document directory.
allinputs = $(mho)tasks/$(MHO)_inputs.txt \
	$(mho)tasks/$(MHO)_0000.txt $(mho)tasks/$(MHO)_ZZZZ.txt \
	$(mho)tasks/$(MHO)_0001.txt $(mho)tasks/$(MHO)_0010.txt \
	$(mho)tasks/$(MHO)_0100.txt $(mho)tasks/$(MHO)_0200.txt \
	$(mho)tasks/$(MHO)_0300.txt $(mho)tasks/$(MHO)_0305.txt \
	$(mho)tasks/$(MHO)_0307.txt $(mho)tasks/$(MHO)_0310.txt \
	$(mho)tasks/$(MHO)_0311.txt $(mho)tasks/$(MHO)_0312.txt \
	$(mho)tasks/$(MHO)_0313.txt $(mho)tasks/$(MHO)_0314.txt \
	$(mho)tasks/$(MHO)_0315.txt $(mho)tasks/$(MHO)_0316.txt \
	$(mho)tasks/$(MHO)_0330.txt $(mho)tasks/$(MHO)_0331.txt \
	$(mho)tasks/$(MHO)_0332.txt $(mho)tasks/$(MHO)_0333.txt \
	$(mho)tasks/$(MHO)_0334.txt $(mho)tasks/$(MHO)_0335.txt \
	$(mho)tasks/$(MHO)_0336.txt $(mho)tasks/$(MHO)_0337.txt \
	$(mho)tasks/$(MHO)_0338.txt \
	$(mho)tasks/$(MHO)_0350.txt $(mho)tasks/$(MHO)_0370.txt \
	$(mho)tasks/$(MHO)_0380.txt $(mho)tasks/$(MHO)_0390.txt \
	$(mho)tasks/$(MHO)_0400.txt $(mho)tasks/$(MHO)_0500.txt \
	$(mho)tasks/$(MHO)_bubbles.pl $(mho)tasks/$(MHO)_config.pl

# the perl scripts that run the task machinery
taskingscripts = taskscripts/sw_tasks.pl taskscripts/sw_task_graph.pl \
	taskscripts/sw_task_mjd.pl taskscripts/sw_task_parser.pl \
	taskscripts/sw_task_timeline.pl taskscripts/sw_task_wbs.pl \
	taskscripts/sw_task_config.pl taskscripts/sw_task_latex.pl \
	taskscripts/sw_task_split.pl

noinst_SCRIPTS = overview-helper.sh plantuml-class-transcriber.py \
	plantuml-gantt-transcriber.py
latexinputs = $(srcdir)/overview.tex

# inputs to the objects section
overviewobjects = domainlist.tex devels.tex taskdefs.tex dates.tex
texincobjects = $(overviewobjects) \
	plantuml-code.uml plantuml-code.png \
	plantuml-code-gantt.uml plantuml-code-gantt.png

EXTRA_DIST = $(srcdir)/$(TEXSRC).tex $(latexinputs) \
	$(appsamples) $(mhoinputs) $(allinputs) $(taskingscripts) \
	$(texincobjects)

if HAVE_PERL
if HAVE_DOT
# this pulls everything in from source dir if we are in build dir
# note that the dependency is on all possible inputs, but the rsync
# will only pull in the group selected by $(mho)
rsopts = -v --exclude '.*.swp'
sync: $(taskingscripts) $(allinputs)
	[ -f Makefile.am ] || \
	rsync -a $(rsopts) $(srcdir)/taskscripts/ $(srcdir)/$(mho)tasks/ .
	touch sync

# this builds everything
REPORT = all
TARGET = ALL
DOTDEV = png # pdf
FLAGS = -v -l -t # very latex things-also
mhoarg = $(FLAGS) -d ${DOTDEV} -i $(MHO).txt  -b $(MHO)_bubbles.pl \
	 -c $(MHO)_config.pl -r $(REPORT) -g $(TARGET)
# now the target: concatenate inputs and run the beast
# fruit:	Makefile sync
fruit:	sync
	-[ $(mho) = all ] && cat $(MHO)_[0Z]*.txt > $(MHO).txt
	./sw_tasks.pl $(mhoarg) -o fruit/$(mho)
	[ -d fruit ] && lines=`ls fruit | wc -l` && [ $$lines -gt 0 ]

# this installs the 'new' task file to the srcdir
#cp -p fruit/$(mho)*.new $(srcdir)/$(mho)tasks
new:	fruit/split
	cp -p fruit/$(MHO)_*.txt $(srcdir)/$(mho)tasks

fruit/split:	sync
	./sw_task_split.pl -i $(MHO).txt -o fruit
	touch fruit/split

save: fruit
	tar zcf $(srcdir)/save-$(mho)-`date +%Y%m%d`.tgz \
		fruit/$(mho)-* $(MHO)*
	ls -l $(srcdir)/save-$(mho)-*.tgz

else !HAVE_DOT
# no task things can be generated because we have no 'dot'
fruit:
	mkdir fruit
	touch fruit/taskage.tex
endif !HAVE_DOT
else !HAVE_PERL
# no task things can be generated because we have no 'perl'
fruit:
	mkdir fruit
	touch fruit/taskage.tex
endif !HAVE_PERL

# this is cleaner, but makes it harder to use display tools that
# watch files on disks as things disappear.
#rm -rf fruit
tasks:
	$(MAKE) fruit

# these rules have temporary hacks in them:
plantuml-code.uml: $(srcdir)/plantuml-class-transcriber.py
	-$(srcdir)/plantuml-class-transcriber.py
	[ -f $@ ] || touch $@
	ls -l $@
plantuml-code.png: plantuml-code.uml
	-plantuml $<
	[ -f plantuml-code.png ] || ln -s $(srcdir)/taskscripts/sw_tasks.png $@
	ls -l $@

plantuml-code-gantt.uml: $(srcdir)/plantuml-gantt-transcriber.py
	-$(srcdir)/plantuml-gantt-transcriber.py
	[ -f $@ ] || touch $@
	ls -l $@
plantuml-code-gantt.png: plantuml-code-gantt.uml
	-plantuml $<
	[ -f plantuml-code-gantt.png ] || ln -s $(srcdir)/taskscripts/sw_tasks.png $@
	ls -l $@

# build objects needed by the document through $(texincobjects)
$(overviewobjects) : texs
texs:	$(srcdir)/overview-helper.sh
	for t in $(overviewobjects);do $(srcdir)/overview-helper.sh $$t;done
	touch texs

# FIXME: this is really a maintainer document that
# perhaps should not be part of the distribution.
if WANT_DOCS
# to be somewhat surgical, this is useful:
# TEXINPUTS=... kpsewhich -show-path tex "filename"
# KPSETEXMFPATH set to : in configure pulls in the rest of TeX
TEXINPUTS=.:fruit:$(srcdir):$(srcdir)/../common//@KPSETEXMFPATH@
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
#$(TEXSRC).pdf: bibs
$(TEXSRC).pdf: force
	@echo final iteration
	-TEXINPUTS=$(TEXINPUTS) pdflatex $(TEXOPT) $(TEXSRC).tex
	mv tasks.log tasks-final.log

# see which backend was requested and invoke the appropriate bib agent
#bibs: force
# @echo biber or biblatex
# -grep -q backend=biber $(srcdir)/$(TEXSRC).tex && \
# 	TEXBIB=$(TEXBIB) biber $(TEXSRC) >$(TEXSRC)-bibs.log || \
# 	BIBINPUTS=$(TEXBIB) bibtex $(TEXSRC) >$(TEXSRC)-bibs.log
# touch bibs

# a first iteration starts the ball rolling
#force: $(srcdir)/Makefile.am $(EXTRA_DIST)
force: fruit $(EXTRA_DIST) texs
	@echo first iteration
	-TEXINPUTS=$(TEXINPUTS) pdflatex $(TEXOPT) $(TEXSRC).tex
	mv tasks.log tasks-force.log
	touch force

CLEANFILES = $(TEXSRC).* $(TEXPDF).pdf force bibs *.log *-blx.bib \
	sync sw_task* $(MHO)* .*.swp save-*tgz tasks-generation* *.txt \
	texs *.tex *.uml *.png *.dot

clean-local:
	-rm -rf fruit

else
# nothing built, installed or cleaned
endif

#
# eof
#
