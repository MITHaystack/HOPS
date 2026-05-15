#!/bin/bash
#
# Script to verify GSL fit example
[ -f cbsplinelsqex.log ] || {
    echo No execution of cbsplinelsqex present.  Skipping out.
    exit 77
}
[ -z "$srcdir" ] && {
    echo no srcdir provided, this is a serious issue
    exit 99
}
grep breakpoints: cbsplinelsqex.log |\
cmp - $srcdir/gslcheck/gslspline.out &&
    echo compiled output agrees with source output ||
    exit 1

grep '^data:'   cbsplinelsqex.log | cut -c 6- >  cbsdata.txt
echo                                          >> cbsdata.txt
echo                                          >> cbsdata.txt
grep '^spline:' cbsplinelsqex.log | cut -c 8- >> cbsdata.txt

# now make a plot
gnuplot=`type -p gnuplot`
[ -x "$gnuplot" ] || {
    echo no gnuplot to plot the data
    exit 77
}

cat > cbsdata.gnu <<EOF
    set term pdfcairo size 8,5
    set output 'cbsdata.pdf'
    set xlabel 'x'
    set xrange [0:15]
    set yrange [-1.5:1.5]
    set grid
    plot 'cbsdata.txt' in 0 u 1:2 with poin ps .5 pt 22 lc 22 tit 'Data', \
         'cbsdata.txt' in 1 u 1:2 with line ls 7 tit 'Spline 40 breaks' , \
         'cbsdata.txt' in 1 u 1:3 with line ls 2 tit 'Spline 10 breaks'
    set output
EOF
gnuplot cbsdata.gnu
[ -f cbsdata.pdf ]
#
# eof vim:nospell
#
