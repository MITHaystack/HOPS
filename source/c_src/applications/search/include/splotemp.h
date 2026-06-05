/*
 * This file holds templates used to construct the
 * eventual gnuplot splot command; the idea is similar
 * to what is done in cohfit, but it is a simpler plot.
 */
#if BIGGER
#ifndef __splotemp_h__
#define __splotemp_h__

/* pdfcairo terminal setup */
#define GNUPLOT_PDFILE "\
# comment-out the next two lines and you can load into gnuplot and\n\
# then also comment out both 'set output' lines around the splot cmd\n\
trmsize=%lf; xtrmsize=trmsize*%lf; ytrmsize=trmsize*%lf\n\
set term pdfcairo size xtrmsize,ytrmsize font '%s,%d'\n\
set title \"Fringe: %s\\n%s\" font '%s,%d' offset 0,1\n\
"

/* pre-command configuration */
#define GNUPLOT_CONFIG "\
#\n\
set key off\n\
set view map\n\
set isosamples %d\n\
set dgrid3d %d,%d %s %d # N,M values\n\
set contour base\n\
set cntrparam bspline order %d\n\
set cntrparam firstlinetype 7\n\
if (%d == 2) {\n\
set cntrparam levels incremental %lf, %lf\n\
} else {\n\
set cntrparam levels auto\n\
}\n\
set pm3d at b\n\
# function for linear in t from mn to mx in nn steps\n\
ldr(sf, t, mn,pk,mx,nn) = \\\n\
    sf*((mn-pk)*((nn-1-t)/(nn-1)) + (mx-pk)*(t/(nn-1)))\n\
f(x) = ldr(%lf, x, %lf,%lf,%lf,%d) # rate  scaling function on 0:N-1\n\
g(y) = ldr(%lf, y, %lf,%lf,%lf,%d) # delay scaling function on 0:M-1\n\
set xrange [f(0):f(%d)]\n\
set xlabel 'Resid.Rate (%s) from Peak Rate (at %.3f %s)'\n\
set yrange [g(0):g(%d)]\n\
set ylabel 'Resid.Delay (%s) from Peak Delay (at %.3f %s)'\n\
set zrange [%lf:%lf]\n\
set cblabel '%s'\n\
set palette %s %s\n\
#\n\
"

/* splot command */
#define GNUPLOT_SPLOT "\
set output '%s'\n\
# disable drawing the lines of the surface which is flat\n\
unset surface\n\
set object 1 rect from graph 0,0 to graph 1,1 behind fc rgb '%s'\n\
set object 2 circle center %lf,%lf size screen %lf \\\n\
  front fc rgb '%s' fs transparent solid %lf\n\
splot '%s' using (f($1)):(g($2)):3 with lines\n\
set output\n\
#\n\
# eof\n\
#\n\
"

#endif /* __splotemp_h__ */
#endif /* BIGGER */
/*
 * end vim:nospell
 */
