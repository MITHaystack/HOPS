#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_ffplot.sh'
me=`basename $0 2>&-` || me=sh
[ "$me" = import_hops.sh ] && return=exit || return=return

CHKSUM=0
if [ "$1" == "--checksum-only" ]; then
	CHKSUM=1
fi

ret_val=0

if [ -z ${HOPS3_SRC_DIR} ] && [ -z ${HOPS4_SRC_DIR} ]; then
    echo "Need to set HOPS3_SRC_DIR and HOPS4_SRC_DIR"
    ret_val=1
else
    [ -z "$bsi" ] && bsi=${HOPS4_SRC_DIR}/data/bootstrap/import_scripts

    #list of plotting header files
    declare -a source_list=(
        "meta_struct.h"
        "plot_data_dir.h"
    )

    src_dir="${HOPS3_SRC_DIR}/postproc/fourfit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/fourfit_libs/ffplot/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    #"print_at.c"
    # "pwrt_left.c"
    # "pwrt_right.c"
    # list of plotting sources
    declare -a source_list=(
    "put_char_at.c"
    "make_plotdata.c"
    "make_postplot.c"
    "plot_complex.c"
    "plot_graph.c"
    "generate_graphs.c"
    "generate_text.c"
    "grid.c"
    "sprint_char_arr.c"
    "sprint_date.c"
    "output.c"
    "display_fplot.c"
    "plot_data_dir.c"
    "fit_vbp.c"
    )

    src_dir="${HOPS3_SRC_DIR}/postproc/fourfit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/fourfit_libs/ffplot/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

fi

$return ${ret_val}
#
# eof
#
