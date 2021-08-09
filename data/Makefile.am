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
	MHO_REGRESSION_TIDY=true

TESTS = switches/template.sh switches/chk_template.sh

# this is just to list what is here
nodist_check_SCRIPTS = \
	bootstrap/import_check.sh \
	bootstrap/import_3to4.sh \
	bootstrap/legacy_tar.sh \
	bootstrap/legacy_nuke.sh \
	bootstrap/req_report.sh
dist_check_SCRIPTS = \
	bootstrap/legacy_update.sh \
	switches/template.sh \
	switches/lookup.sh

BUILT_SOURCES = switches/chk_template.sh

switches/chk_template.sh: $(srcdir)/switches/template.sh $(srcdir)/Makefile.am
	-rm -f $@
	[ -d switches ] || mkdir switches
	cd switches && $(srcdir)/switches/template.sh template
	sed -e '/tarballs=/s/FIXME//' \
	    -e '/executables=/s/FIXME/true false/' \
	    -e '/^..FIXME...whatever/s/^..FIXME/$$true \&\& echo true ok/' \
	    -e '/^..FIXME...however/s/^..FIXME/$$false || echo false ok/' \
	    -e '/^.true/s/..whatever/; [ -x $$true ] || passfail=1/' \
	    -e '/^.false/s/..however/; [ -x $$false ] || passfail=2/' \
	    $@ > $@.tmp && \
	mv $@.tmp $@ && chmod +x $@

EXTRA_DIST = bootstrap/requirements.txt

CLEANFILES = chk_template.sh

#
# eof
#
