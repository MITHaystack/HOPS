#!/bin/bash
#ffcore is responsible for managing the initialization and organization
#of the pass and param structures

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_ffcore.sh'
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

    #list of core fourfit header files
    declare -a source_list=(
    "filter.h"
    "ff_misc_if.h"
    "freqlist.h"
    "param_struct.h"
    "pass_struct.h"
    "plot_struct.h"
    "refringe.h"
    "statistics.h"
    )

    src_dir="${HOPS3_SRC_DIR}/postproc/fourfit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/fourfit_libs/ffcore/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    #list of core fourfit source files moving from include to ffcore
    declare -a source_list=(
        "fourfit_signal_handler.h"
        "write_lock_mechanism.h"
    )

    src_dir="${HOPS3_SRC_DIR}/include"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/fourfit_libs/ffcore/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    # list of core fourfit source files
    declare -a source_list=(
    "create_lockfile.c"
    "fourfit_signal_handler.c"
    "wait_for_write_lock.c"
    "clear_pass.c"
    "make_flist.c"
    "make_passes.c"
    "fill_param.c"
    "read_sdata.c"
    "time_range.c"
    "get_corel_data.c"
    "set_defaults.c"
    "clear_freq_corel.c"
    "check_rflist.c"
    "refringe_list.c"
    "set_pointers.c"
    "generate_cblock.c"
    "vrot.c"
    "pcal_interp.c"
    )

    src_dir="${HOPS3_SRC_DIR}/postproc/fourfit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/fourfit_libs/ffcore/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))
fi

$return ${ret_val}
#
# eof
#
