#!/bin/bash
#
# Script to verify GSL bspline LSQ example output.
#
# Usage: cbsplinecheck.sh <variant>
#   <variant> is "2p7" or "2p8" -- selects the binary and reference file.
#
variant="${1:-2p8}"
log="cbsplinelsqex${variant}.log"
[ -f "$log" ] || {
    echo No execution of cbsplinelsqex${variant} present.  Skipping out.
    exit 77
}
[ -z "$srcdir" ] && {
    echo no srcdir provided, this is a serious issue
    exit 99
}
ref="$srcdir/gslspline${variant}.output"
[ -f "$ref" ] || { echo Missing reference $ref ; exit 99; }

grep breakpoints: "$log" | cmp - "$ref" &&
    echo compiled output agrees with source output ||
    exit 1

# now make a plot (best-effort, skip if no gnuplot)
gnuplot=`type -p gnuplot`
[ -x "$gnuplot" ] || {
    echo no gnuplot to plot the data
    exit 0
}

grep '^data:'   "$log" | cut -c 6- >  cbsdata${variant}.txt
echo                                  >> cbsdata${variant}.txt
echo                                  >> cbsdata${variant}.txt
grep '^spline:' "$log" | cut -c 8- >> cbsdata${variant}.txt

cat > cbsdata${variant}.gnu <<EOF
    set term pdfcairo size 8,5
    set output 'cbsdata${variant}.pdf'
    set xlabel 'x'
    set xrange [0:15]
    set yrange [-1.5:1.5]
    set grid
    plot 'cbsdata${variant}.txt' in 0 u 1:2 with poin ps .5 pt 22 lc 22 tit 'Data', \
         'cbsdata${variant}.txt' in 1 u 1:2 with line ls 7 tit 'Spline 40 breaks' , \
         'cbsdata${variant}.txt' in 1 u 1:3 with line ls 2 tit 'Spline 10 breaks'
    set output
EOF
gnuplot cbsdata${variant}.gnu
[ -f cbsdata${variant}.pdf ]
#
# eof vim:nospell
#
