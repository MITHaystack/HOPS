#!/bin/sh
#
# Trivial formatter
#
# Input is a list of test log files and the result of grepping for
# REQUIREMENTS in each one.  (CMake and Automake present results
# differently, so the grepping script will solve that problem.)
#
# Expect something like the following...
#  ./data/chk_lookup.sh.log
#  ./data/chk_template.sh.log
#  ./data/chk_unpack.sh.log
#  ./source/c_src/applications/test/chk_2836.sh.log
#  REQUIREMENTS: 2836 S-1 0
#
#                name req status
#
# and generate a pretty document.
#
echo '###############################################################'
echo "## Requirements status $1 ##"
echo '###############################################################'
grep '^REQUIREMENTS'
echo '###############################################################'
#
# eof
#
