#!/bin/bash
#
# simple script to avoid having to cram things in Makefile language
#
out=${1-'error.tex'}
err='error.tex'
in=${2-'MHO_task.txt'}
hl="-------------------------------------------------------------"
hl=$hl"----------------------------------------------------------"
rm -f $out
echo "$1 $2" >> $err
case $out in
domainlist.tex)
    echo '\begin{verbatim}'
    ./sw_tasks.pl -g help -i MHO_task.txt |\
        sed -e '/All.output.appears/d' \
            -e '/Available domains to/d'
    echo '\end{verbatim}'
    ;;

abbrev.tex)     # {Some Tasking Abbreviations}
    echo '\begin{tabbing}'
    echo 'xxxxxxxxxxxxx \= xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \kill'
    sed -e '/^### some common abbreviations/,/^$/!d' $in |\
    grep -v '^### some common abbreviations'  |\
    sed -e 's/=/\\>/' -e 's/$/  \\\\/'
    echo '\end{tabbing}'
    ;;

dates.tex)      # {Fixed Dates}
    echo '\begin{tabbing}'
    echo "$hl\\\\"
    echo 'xxxxxxxx \= xxxxxxxxxxx \= xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \kill'
    sed -e '/^### Start of timeline abbreviations/,/^$/!d' $in |\
    grep -v '^### Start of timeline abbreviations'  |\
    sed -e 's/@//' -e '/^#/s/^.*$/'"$hl"'/' \
        -e 's/=/\\>/' -e 's/  #/\\>/' -e 's/$/  \\\\/' |\
    uniq | grep -v '^....$'
    echo "$hl\\\\"
    echo '\end{tabbing}'
    ;;

devels.tex)     # {Pool of Developers and Other Relevant People}
    echo '\begin{tabbing}'
    echo 'xxxxxxxxxxxxx \= xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \kill'
    echo "$hl\\\\"
    sed -e '/^### pool of developers and other people/,/^$/!d' $in |\
    grep -v '^### pool of developers and other people' |\
    sed -e '/^#/s/^.*$/'"$hl"'/' \
        -e 's/=/\\>/' -e 's/$/  \\\\/'
    echo "$hl\\\\"
    echo '\end{tabbing}'
    ;;

taskdefs.tex)   # {Task Definitions}
    echo '\begin{tabbing}'
    echo 'xxxxxxxxxxxxx \= xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \kill'
    #echo "$hl\\\\"
    sed -e '/^### some common task definitions/,/^# eof/!d' $in |\
    grep -v '^### some common task definitions' |\
    grep -v '^# note that after using these to set' |\
    grep -v '^# then refine effort' |\
    grep -v '^# eof' |\
    sed -e '/^#/s/^#/ \\>/' -e '/^$/s/^/'"$hl"'/' \
        -e 's/=/\\>/' -e 's/$/  \\\\/'
    echo '\end{tabbing}'
    ;;

*)
    echo "$out" not configured 1>&2
    ;;
esac > $out 2>>$err
ls -l $out
#
# eof
#
