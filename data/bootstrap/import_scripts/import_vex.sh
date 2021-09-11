#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_vex.sh'
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

    #list of vex header files
    declare -a source_list=(
        "cvex.h"
        "evex.h"
        "ivex.h"
        "lvex.h"
        "mk4_vex.h"
        "ovex.h"
        "svex.h"
        "vex.h"
    )

    src_dir="${HOPS3_SRC_DIR}/include"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/vex/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    #list of vex source files
    declare -a source_list=(
        "block_params.c"
        "check_intrange.c"
        "check_realrange.c"
        "check_scan.c"
        "check_stloc.c"
        "check_strrange.c"
        "cvex_info.c"
        "decode_pval.c"
        "do_antenna.c"
        "do_bbc.c"
        "do_clock.c"
        "do_das.c"
        "do_eop.c"
        "do_exper.c"
        "do_freq.c"
        "do_head_pos.c"
        "do_if.c"
        "do_pass_order.c"
        "do_phase_cal_detect.c"
        "do_roll.c"
        "do_site.c"
        "do_source.c"
        "do_track.c"
        "evex_info.c"
        "fill_deflists.c"
        "fill_scanlist.c"
        "fill_station_parms.c"
        "find_statements.c"
        "get_block_mode.c"
        "get_chip_mode.c"
        "get_corr_bd_parms.c"
        "get_corr_mode.c"
        "get_def.c"
        "get_drive_init.c"
        "get_global_deflist.c"
        "get_logscan.c"
        "get_mode_deflist.c"
        "get_pbs_init.c"
        "get_pcm_config.c"
        "get_pcm_tables.c"
        "get_section_mode.c"
        "get_statement.c"
        "get_station_deflist.c"
        "get_su_chan_out.c"
        "get_su_connect.c"
        "get_trm_config.c"
        "get_val_list.c"
        "get_version.c"
        "get_vex.c"
        "in_comment.c"
        "init_scan.c"
        "in_quote.c"
        "ivex_info.c"
        "locate_blocks.c"
        "locate_cq.c"
        "lvex_info.c"
        "nextchar.c"
        "param_formats.c"
        "parse_date.c"
        "parse_dec.c"
        "parse_pval.c"
        "parse_ra.c"
        "parse_ref.c"
        "parse_units.c"
        "parse_vexfile.c"
        "print_location.c"
        "process_qstring.c"
        "read_file.c"
        "scan_info.c"
        "strip_text.c"
        "svex_info.c"
        "vex_init.c"
        "write_vexfile.c"
    )

    src_dir="${HOPS3_SRC_DIR}/sub/vex"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/vex/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))
fi

$return ${ret_val}
#
# eof
#
