#
# Makefile for data and integration tests
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
SUBDIRS = 

TESTS_ENVIRONMENT = srcdir=$(srcdir)

TESTS = switches/template.sh

# this is just to list what is here
nodist_check_SCRIPTS = \
	bootstrap/import_check.sh \
	bootstrap/import_3to4.sh \
	bootstrap/legacy_tar.sh \
	bootstrap/legacy_nuke.sh \
	bootstrap/req_report.sh
dist_check_SCRIPTS = \
	bootstrap/legacy_update.sh \
	switches/template.sh

EXTRA_DIST = bootstrap/requirements.txt

CLEANFILES = chk_template.sh

#
# eof
#
