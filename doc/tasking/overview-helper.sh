#!/bin/bash
#
# simple script to avoid having to cram things in Makefile language
#
out=${1-'error.tex'}
err='error.tex'
in=${2-'MHO_task.txt'}
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
devels.tex)     # {Pool of Developers and Other Relevant People}
    echo '\begin{tabbing}'
    echo 'xxxxxxxxxxxxx \= xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \kill'
    sed -e '/^### pool of developers and other people/,/^$/!d' $in |\
    grep -v '^### pool of developers and other people' |\
    sed -e '/^#/s/^.*$/\\hline /' \
        -e 's/=/\\>/' -e 's/$/  \\\\/'
    echo '\end{tabbing}'
    ;;

taskdefs.tex)   # {Task Definitions}
    echo '%'
    ;;

dates.tex)      # {Fixed Dates}
    echo '%'
    ;;
*)
    echo "$out" not configured 1>&2
    ;;
esac > $out 2>>$err
ls -l $out
#
# eof
#
