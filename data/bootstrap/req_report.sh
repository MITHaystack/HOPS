#!/bin/sh -x
#
# Gather information for a report.
#
dir=${1-'.'}
rep=${2-'cat'}

# canonical choice
[ -n "$MHO_REGRESSION_DATA" -a -d "$MHO_REGRESSION_DATA" -a "$rep" = cat ] && {
    xrep=$MHO_REGRESSION_DATA/bootstrap/reporter.sh
    [ -x $xrep ] || {
        echo The canonical reporter does not exist.
        echo This is a problem.
        exit 2
    }
} || {
    xrep=`type -p $rep`
    [ -x $xrep ] || {
        echo Usage: $0 directory reporter
        echo
        echo Which searches the named directory for check tests
        echo and passes results to the reporter for formatting.
        exit 1
    }
}

# make it pretty
timest=`date -u +%Y-%m-%d`
banner="$timest                           "

# look for the canonical check scripts
find $dir -name chk_*.sh.log -print -exec grep '^REQUIREMENTS:' {} \; |\
grep -v template |\
$xrep "$banner"

exit 0
#
# eof
#
