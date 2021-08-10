#!/bin/sh
#
# Trivial formatter
#
# Expect something like the following...
#  ./data/chk_lookup.sh.log
#  ./data/chk_template.sh.log
#  ./data/chk_unpack.sh.log
#  ./source/c_src/applications/test/chk_2836.sh.log
#  REQUIREMENTS: 2836 S-1 0
#
# and generate a pretty document.
#
echo '###############################################################'
echo '## Requirements status as of '`date -u`' ##'
echo '###############################################################'
grep ^REQUIREMENTS
echo '###############################################################'
#
# eof
#
