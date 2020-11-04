#!/bin/bash
#
# helper script to generate coverage reports
# it is invoked by the automake coverage-local target with
#   $(top_srcdir)/coverep.sh $(GCOV) $(go) $(abs_srcdir) $(abs_builddir)
#
echo \
'============================================================================'
cov=$1
opt=$2
src=$3
bld=$4
top=$5
[ $# -ne 5 ] && cat <<....EOF

Usage: $0 gcov options abssrcdir topblddir topsrcdir

    This uses the coverage files *.gcno *.gcda *.gcov generated in
    compilation and execution, and provides 4 reports:
    
        coverage-files.txt      object files to consider
        coverage-report.txt     raw output from gcov
        coverage-todo.txt       a mangled cull from the report
        coverage-summary.txt    a readable version

    Additionally, you can set environment variables:
        export GCOV_SUMMARY_WIDTH=xx to widen the report
        export GCOV_LOCAL_ONLY=true to limit to local functions
    'see gcov(1) for explanation of gcov options and gcc(1)'

    Details on coverage are found in the individual .gcov files.

This script must be invoked from the Makefile with 5 arguments.

....EOF
[ $# -ne 5 ] && exit 0

[ -x "$cov" ] || { echo no coverage executable "'$cov'"; exit 2; }
[ -d "$src" ] || { echo no abs source dir "'$src'"; exit 3; }
[ -d "$bld" ] || { echo no abs build dir "'$bld'"; exit 4; }
[ -d "$top" ] || { echo no top source dir "'$top'"; exit 5; }
echo "Generating coverage in $bld"
echo " from sources found in $src"
echo \
'============================================================================'

targets="coverage-report.txt coverage-todo.txt coverage-summary.txt"
targets="$targets coverage-files.txt"
rm -f $targets

# generate a list of files we should be covering
[ -f $src/coverage-files.txt ] && cp -p $src/coverage-files.txt . ||
    find . -name \*.o | sed 's/.o$//' > coverage-files.txt

# generate the detailed coverage report
for tt in `cat coverage-files.txt`
do $cov $opt $tt >> coverage-report.txt 2>&1 ; done

# sine qua non
[ -s coverage-report.txt ] || {
    cat<<....EOF
    ###################################################
    # No Coverage report found -- considering this to #
    # be an accidental invocation and are moving on   #
    ###################################################
....EOF
    exit 0
}

# cull for only what is partially covered
egrep '^(Func|File|Lines|No.*lines)' coverage-report.txt |\
    paste - - | grep -v ':100.00' | grep -v ':0.00%' |\
    grep -v 'No executable lines' |\
    sed -e "s+$src+.+" \
        -e "/File/s+.c'+.c.gcov'+" \
        -e "s+$top+(top)+" \
        -e 's+/usr/include/c+(cpp)+' \
        > coverage-todo.txt

# it is hard to get the width correct automatically
[ -n "$GCOV_SUMMARY_WIDTH" ] && wid=${GCOV_SUMMARY_WIDTH}s || wid=55s
[ "x$GCOV_LOCAL_ONLY" = xtrue ] && {
    mv coverage-todo.txt coverage-temp.txt
    egrep -v '(\(top\)|\(cpp\))' coverage-temp.txt > coverage-todo.txt
}

# and generate a pretty, readable summary of that.
grep -v '0.00%' coverage-todo.txt |\
    awk '{printf "%11s %-'$wid' %s %s %s %s\n",$1,$2,$3,$4,$5,$6}' \
        > coverage-summary.txt
grep 'assuming.not.executed' coverage-report.txt |\
    sed 's/^/  /' >> coverage-summary.txt

[ -s coverage-summary.txt ] && cat coverage-summary.txt
echo \
'============================================================================'

# no work equals no problem
[ -s coverage-files.txt ] || exit 0

# no report, however is.
[ -s coverage-report.txt ]
#
# eof
#
