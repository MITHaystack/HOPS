#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_aedit.sh'
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

    # aedit include files
    declare -a source_list=(
        "aedata.h" 
        "aedit.h" 
        "aedit_proto.h" 
        "close_flags.h" 
        "flags.h" 
        "param_list.h" 
        "psplot.h" 
        "pstruct.h" 
        "sizelimits.h" 
        "sort.h" 
        "summary.h" 
        "tempo.h" 
        "usearray.h" 
    )
    
    src_dir="${HOPS3_SRC_DIR}/postproc/aedit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/applications/aedit/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    # aedit application (library) sources
    declare -a source_list=(
    "active_filter.c" 
    "add_station.c" 
    "aeclr_corel.c" 
    "aeclr_fringe.c" 
    "aeclr_quad.c" 
    "aeclr_root.c" 
    "aeclr_triangle.c" 
    "allocate_parms.c" 
    "alloc_btq.c" 
    "auto_hardcopy.c" 
    "axis.c" 
    "axis_scale.c" 
    "calc_close.c" 
    "cfilter.c" 
    "check_env.c" 
    "cleanup.c" 
    "cleanup_psplot.c" 
    "clear.c" 
    "clear_fqex.c" 
    "clear_pstruct.c" 
    "clear_source.c" 
    "clear_summ.c" 
    "command_id.c" 
    "cross_link.c" 
    "cursor_select.c" 
    "datum_value.c" 
    "display_psdata.c" 
    "dup_check.c" 
    "dup_flag.c" 
    "edit.c" 
    "edit_close.c" 
    "edit_families.c" 
    "erase_point.c" 
    "esdesp_check.c" 
    "execute.c" 
    "extract_parms.c" 
    "ffilter.c" 
    "fill4_parms.c" 
    "fill_closure.c" 
    "fill_tdata.c" 
    "fplot.c" 
    "get_axis.c" 
    "get_param.c" 
    "get_param_list.c" 
    "get_plot_data.c" 
    "get_plot_datum.c" 
    "get_ps_indices4.c" 
    "help.c" 
    "init_inputs.c" 
    "init_summ.c" 
    "locate_pscurs.c" 
    "makekey.c" 
    "make_psarray4.c" 
    "make_tri.c" 
    "mk3_qf.c" 
    "param_list.c" 
    "parse_cmdline.c" 
    "parse_commands.c" 
    "plot.c" 
    "plot_fqex.c" 
    "plot_header.c" 
    "plot_points.c" 
    "plot_quality.c" 
    "pr_allsumm.c" 
    "pr_csumm.c" 
    "pr_fsumm.c" 
    "pr_inputs.c" 
    "print_data.c" 
    "pr_rsumm.c" 
    "pr_source.c" 
    "pr_ssumm.c" 
    "pr_summary.c" 
    "pr_tsumm.c" 
    "ps_baselabel.c" 
    "psfile4.c" 
    "ps_fplot.c" 
    "ps_free.c" 
    "ps_inside.c" 
    "psplot4.c" 
    "psplot.c" 
    "psplot_defaults4.c" 
    "ps_proc_datum.c" 
    "ps_scanlabel.c" 
    "ps_selbase.c" 
    "ps_selqual.c" 
    "ps_selscan.c" 
    "pstag_process.c" 
    "qarray_index.c" 
    "qfilter.c" 
    "read_cursor.c" 
    "read_data.c" 
    "rfilter.c" 
    "run_com_file.c" 
    "run_pscursor.c" 
    "save.c" 
    "set_baselines.c" 
    "set_device.c" 
    "set_fraction.c" 
    "set_frequencies.c" 
    "set_mode.c" 
    "set_nfreq.c" 
    "set_pols.c" 
    "set_prange.c" 
    "set_procrange.c" 
    "set_pscodes.c" 
    "set_psparam.c" 
    "set_qcodes.c" 
    "set_quads.c" 
    "set_sources.c" 
    "set_stations.c" 
    "set_timerange.c" 
    "set_triangles.c" 
    "set_type.c" 
    "setup_plot.c" 
    "setup_psplot.c" 
    "set_year.c" 
    "smatch.c" 
    "sorter.c" 
    "station_reduce.c" 
    "summ_corel.c" 
    "summ_data.c" 
    "summ_fringe.c" 
    "summ_quad.c" 
    "summ_root.c" 
    "summ_triangle.c" 
    "symbol_key.c" 
    "tarray_index.c" 
    "test1.c" 
    "tfilter.c" 
    "time_axis.c" 
    "triangle_value.c" 
    "tricheck.c" 
    "trngl_present.c" 
    "unflag.c" 
    "update_fqex.c" 
    "update_sinfo.c" 
    "write_data.c" 
    "write_families.c" 
    "write_param.c" 
    "write_prmline.c" 
    "write_psfile.c" 
    "write_pshdr.c" 
    "write_reproc.c" 
    "write_tdata.c" 
    "zoom.c" 
    )
    
    src_dir="${HOPS3_SRC_DIR}/postproc/aedit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/applications/aedit/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    declare -a source_list=( aedit.c )
    src_dir="${HOPS3_SRC_DIR}/postproc/aedit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/applications/aedit"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))
fi

$return ${ret_val}
#
# eof
#
