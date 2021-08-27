#
# Makefile for data and integration tests
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
SUBDIRS = 

# the first three are commonly useful
# the second three allow some primitive machinery checking
# NOTE: since we set MHO_REGRESSION* here, this ONE directory cannot
# actually make use of any real data other than what is in source.
TESTS_ENVIRONMENT = \
	abs_top_builddir=$(abs_top_builddir) \
	abs_top_srcdir=$(abs_top_srcdir) \
	srcdir=$(srcdir) \
	MHO_REGRESSION_DATA=$(srcdir) \
	MHO_REGRESSION_EXTRACT=true \
	MHO_REGRESSION_TIDY=true \
	MHO_REGRESSION_FAKE=.

TESTS = template.sh chk_template.sh chk_unpack.sh chk_lookup.sh

check_PROGRAMS = template.sh chk_template.sh chk_unpack.sh

# this is just to list what is here
nodist_check_SCRIPTS = \
	bootstrap/import_check.sh \
	bootstrap/import_3to4.sh \
	bootstrap/legacy_tar.sh \
	bootstrap/legacy_nuke.sh \
	bootstrap/req_report.sh 
dist_check_SCRIPTS = \
	bootstrap/legacy_update.sh \
	bootstrap/legacy_unpack.sh \
	switches/template.sh \
	switches/lookup.sh

template.sh: $(srcdir)/switches/template.sh $(srcdir)/Makefile.am
	sed -e '/Error:/s/exit.1/echo disabled/' $< > $@ && \
	chmod +x $@

# use sed to make the generated test into a test of executables
assign1= $$true \&\& echo true ok ; [ -x $$true ] || passfail=1
assign2= $$false || echo false ok ; [ -x $$false ] || passfail=2
chk_template.sh: template.sh $(srcdir)/Makefile.am
	rm -f $@ && \
	./template.sh template && \
	sed -e '/tarballs=/s/FIXME//' \
	    -e '/executables=/s/FIXME/true false/' \
	    -e '/FIXME.*whatever/s+.*+$(assign1)+' \
	    -e '/FIXME.*however/s+.*+$(assign2)+' \
	    -e '/passfail=99/s/99/0/' \
	    $@ > $@.tmp && \
	mv $@.tmp $@ && chmod +x $@ && grep ok $@

# use sed to make a test of unpacking, use make macros for clarity
assign3= pdf=$$MHO_REGRESSION_FAKE/ff_testdata/misc/cmp-2843.pdf
assign4= [ -f $$pdf ] || { echo $$pdf missing ; passfail=1 ; }
chk_unpack.sh: template.sh $(srcdir)/Makefile.am
	rm -f $@ && \
	./template.sh unpack && \
	sed -e '/tarballs=/s/FIXME/misc/' \
	    -e '/executables=/s/FIXME//' \
	    -e '/FIXME...whatever/s+.*+$(assign3)+' \
	    -e '/FIXME...however/s+.*+$(assign4)+' \
	    -e '/passfail=99/s/99/0/' \
	    $@ > $@.tmp && \
	mv $@.tmp $@ && chmod +x $@ && grep pdf $@

EXTRA_DIST = bootstrap/requirements.txt

# this may cause problems with make dist
CLEANFILES = template.sh chk_template.sh chk_unpack.sh chk_.sh

clean-local:
	rm -rf ff_testdata

#
# eof
#
