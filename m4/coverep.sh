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
[ $# -ne 5 ] && cat <<EOF

Usage: $0 gcov options abssrcdir topblddir topsrcdir

    This uses the coverage files *.gcno *.gcda *.gcov generated in
    compilation and execution, and provides 4 reports:
    
        coverage-files.txt      object files to consider
        coverage-report.txt     raw output from gcov
        coverage-todo.txt       a mangled cull from the report
        coverage-summary.txt    a readable version

    See gcov(1) for explanation of gcov options and gcc(1).

    Additionally, you can set environment variables to adjust the
    appearance of the summary report (doesn't change gcov handling):
        export GCOV_SUMMARY_WIDTH=xx to widen the width of the report
        export GCOV_LOCAL_ONLY=true to limit to local functions
        export GCOV_NO_SWIG=true to skip SWIG functions
        export GCOV_NO_CPP=true to skip C++ system files
        export GCOV_NO_PYTHON=true to skip Python system files
        export GCOV_NO_EXEC=true to skip mention of 'not executed' things
    Since that is alot to type,
        export GCOV_SIMPLE=true sets all of those and a width of 34
                           which might be clearer.

    Details on coverage are found in the individual .gcov files.

This script must be invoked from the Makefile with 5 arguments.

EOF
[ $# -ne 5 ] && exit 0

[ -x "$cov" ] || { echo no coverage executable "'$cov'"; exit 2; }
[ -d "$src" ] || { echo no abs source dir "'$src'"; exit 3; }
[ -d "$bld" ] || { echo no abs build dir "'$bld'"; exit 4; }
[ -d "$top" ] || { echo no top source dir "'$top'"; exit 5; }
echo "Generating coverage in $bld"
echo " from sources found in $src"
# global knob:
[ "x$GCOV_SIMPLE" = xtrue ] && {
    GCOV_SUMMARY_WIDTH=34
    GCOV_LOCAL_ONLY=true
    GCOV_NO_SWIG=true
    GCOV_NO_CPP=true
    GCOV_NO_PYTHON=true
    GCOV_NO_EXEC=true
    echo " Simplified, wide view options in effect"
}

echo \
'============================================================================'

targets="coverage-report.txt coverage-todo.txt coverage-summary.txt"
targets="$targets coverage-files.txt"
rm -f $targets

# generate a list of files we should be covering
#[ -f $src/coverage-files.txt ] && cp -p $src/coverage-files.txt . ||
#    find . -name \*.o | sed 's/.o$//' > coverage-files.txt
# the above finds objects built for libs
[ -f $src/coverage-files.txt ] && cp -p $src/coverage-files.txt . ||
    find . -name \*.gcda | sed 's/.gcda$//' > coverage-files.txt

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
        -e 's+/usr/include/+(usr)/+' \
        > coverage-todo.txt

# it is hard to get the width correct automatically
[ -n "$GCOV_SUMMARY_WIDTH" ] && wid=${GCOV_SUMMARY_WIDTH}s || wid=55s
[ "x$GCOV_LOCAL_ONLY" = xtrue ] && {
    mv coverage-todo.txt coverage-temp.txt
    egrep -v '(\(top\)|\(cpp\))' coverage-temp.txt > coverage-todo.txt
}
[ "x$GCOV_NO_SWIG" = xtrue ] && {
    mv coverage-todo.txt coverage-temp.txt
    egrep -v 'Function .(SWIG_|Swig)' coverage-temp.txt > coverage-todo.txt
}
[ "x$GCOV_NO_CPP" = xtrue ] && {
    mv coverage-todo.txt coverage-temp.txt
    grep -v "File '.cpp.++/" coverage-temp.txt > coverage-todo.txt
}
[ "x$GCOV_NO_PYTHON" = xtrue ] && {
    mv coverage-todo.txt coverage-temp.txt
    grep -v "File '.usr./python" coverage-temp.txt > coverage-todo.txt
}

# and generate a pretty, readable summary of that.
grep -v '0.00%' coverage-todo.txt |\
    awk '{printf "%11s %-'$wid' %s %s %s %s\n",$1,$2,$3,$4,$5,$6}' \
        > coverage-summary.txt
[ "x$GCOV_NO_EXEC" = xtrue ] || {
    grep 'assuming.not.executed' coverage-report.txt |\
        sed 's/^/  /' >> coverage-summary.txt
}

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
