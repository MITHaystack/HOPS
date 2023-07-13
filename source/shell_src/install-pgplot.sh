#!/bin/sh

# Author: Violet Pfeiffer
# Contact: violetp@mit.edu
# Purpose: Install pgplot and cpgplot
# Usage: sudo ./install-pgplot.sh
# Args: None
# Returns: None

wget "ftp://ftp.astro.caltech.edu/pub/pgplot/pgplot5.2.tar.gz"
tar xf pgplot5.2.tar.gz -C /usr/local/

# Add the following permanently to your .bashrc after installation
PGPLOT_DIR=/usr/local/pgplot
export PGPLOT_DIR
PGPLOT_INCLUDE_DIR=/usr/local/pgplot
export PGPLOT_INCLUDE_DIR
PGPLOT_LIBRARY=/usr/local/pgplot/libpgplot.so
export PGPLOT_INCLUDE_DIR

cd $PGPLOT_DIR
cp $difx_directory/applications/hops/trunk/pgplot-drivers.list .
# Modify gfortran_gcc.conf to use gfortran_gcc instead of g77_gcc
sed s/g77/gfortran/ $PGPLOT_DIR/sys_linux/g77_gcc.conf > $PGPLOT_DIR/sys_linux/gfortran_gcc.conf
# Build pgplot and cpgplot
$PGPLOT_DIR/makemake $PGPLOT_DIR linux gfortran_gcc
make && make cpg
# Copy pgplot.pc and cpgplot.pc to the correct locations. (This must be done manually by the user)
# Ensure that the pgplot.pc cpgplot.cp files are pointing to your $PGPLOT_DIR
# Copy both files to /urs/local/lib/pkgconfig
cd ~

tee > cpgplot.pc << EOF
prefix=/usr
exec_prefix=/usr
libdir=/usr/local/pgplot
includedir=/usr/local/pgplot

Name: cpgplot
Description: C interface of pgplot
Version: 5.2.2
Requires: pgplot
Libs: -L${libdir} -lcpgplot
EOF
tee > pgplot.pc << EOF
prefix=/usr
exec_prefix=/usr
libdir=/usr/local/pgplot
includedir=/usr/local/pgplot

Name: pgplot
Description: Graphic library for making simple scientific graphs
Version: 5.2.2
Libs: -L${libdir} -lpgplot -lgfortran -lm -lX11
#Cflags: -I${includedir}
EOF

mv *pgplot.pc /usr/local/lib/pkgconfig/
