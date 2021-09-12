#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_ffsearch.sh'
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

    #list of search header files
    declare -a source_list=( "adhoc_flag.h" "apply_funcs.h" )

    src_dir="${HOPS3_SRC_DIR}/postproc/fourfit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/fourfit_libs/ffsearch/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    # list of search sources
    #   "report_actions.c"
    declare -a source_list=(
        "adhoc_flag.c"
        "adjust_snr.c"
        "apply_cmplxbp.c"
        "apply_filter.c"
        "apply_notches.c"
        "apply_passband.c"
        "calc_normalization.c"
        "calc_rms.c"
        "compute_model.c"
        "compute_qf.c"
        "freq_spacing.c"
        "fringe_search.c"
        "delay_rate.c"
        "gate_off.c"
        "interp.c"
        "ion_search.c"
        "norm_fx.c"
        "norm_xf.c"
        "organize_data.c"
        "est_pc_manual.c"
        "precorrect.c"
        "pcalibrate.c"
        "rotate_pcal.c"
        "sampler_delays.c"
        "search.c"
        "search_windows.c"
        "update.c"
        "create_fname.c"
        "parse_cmdline.c"
        "apply_video_bp.c"
        "ion_covariance.c"
    )

    src_dir="${HOPS3_SRC_DIR}/postproc/fourfit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/fourfit_libs/ffsearch/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))
fi

$return ${ret_val}
#
# eof
#
