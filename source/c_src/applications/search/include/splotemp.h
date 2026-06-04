/*
 * This file holds templates used to construct the
 * eventual gnuplot splot command; the idea is similar
 * to what is done in cohfit, but it is a simpler plot.
 * Each string defined has a BASE size which is updated
 * for the various %-items expanded.
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
set dgrid3d %d,%d %s %d\n\
set contour base\n\
set cntrparam bspline order %d\n\
set cntrparam firstlinetype 7\n\
set cntrparam levels incremental %lf, %lf\n\
set pm3d at b\n\
set xrange [0:%d]\n\
set xlabel 'Resid.Rate (ps/s) FIXME values'\n\
set yrange [0:%d]\n\
set ylabel 'Resid.Delay (us) FIXME values'\n\
set zrange [%lf:%lf]\n\
set cblabel '%s'\n\
set palette rgbformulae '%s'\n\
#\n\
"

/* splot command */
#define GNUPLOT_SPLOT "\
set output '%s'\n\
# disable drawing the lines of the surface which is flat\n\
unset surface\n\
set object 1 rect from graph 0,0 to graph 1,1 behind fc rgb '%s'\n\
splot '%s' with lines\n\
set output\n\
#\n\
# eof\n\
#\n\
"
//#define GNUPLOT_SPLOT_BASE 300

#endif /* __splotemp_h__ */
#endif /* BIGGER */
/*
 * end vim:nospell
 */
