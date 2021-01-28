#!/bin/bash
#
# This is a placeholder test
#

# testverb is an integer
[ -n "$testverb" -a "$testverb" -gt 0 ] && verb=true || verb=false
$verb && [ "$testverb" -gt 1 ] && vrb1=true || vrb1=false
# ...
$vrb1 && echo being really verbose about commentary here.

[ -n "$srcdir" ] && $verb && echo srcdir is $srcdir
[ -d "$srcdir" -a -f "$srcdir/bootstrap/import_check.sh" ] &&
    status=0 || status=1

exit $status
#
# eof
#
