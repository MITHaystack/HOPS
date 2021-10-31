#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_mk4util.sh'
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

    #list of header files for mk4utils
    declare -a source_list=(
        "account.h"
        "fileset.h"
        "fstruct.h"
        "general.h"
        "mk4_sizes.h"
        "mk4_typedefs.h"
        "mk4_util.h"
        "adler32_checksum.h"
        "hops_complex.h"
    )

    src_dir="${HOPS3_SRC_DIR}/include"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/mk4util/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    # sources for mk4utils
    declare -a source_list=(
        "account.c"
        "adler32_checksum.c"
        "check_name.c"
        "clear_date.c"
        "clear_fstruct.c"
        "confirm.c"
        "datec_to_datef.c"
        "datef_to_datec.c"
        "day_of_datef.c"
        "environment.c"
        "extract_filenames.c"
        "fileset.c"
        "get_abs_path.c"
        "get_filelist.c"
        "hops_complex.c"
        "hptoie4.c"
        "hptoie8.c"
        "hptoie.c"
        "int_to_time.c"
        "ismk4.c"
        "minmax.c"
        "report_times.c"
        "root_belong.c"
        "root_id.c"
        "sexigesimal2hrdeg.c"
        "sort_names.c"
        "swap.c"
        "syntax.c"
        "time_to_double.c"
        "time_to_int.c"
    )

    src_dir="${HOPS3_SRC_DIR}/sub/util"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/mk4util/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

fi

$return ${ret_val}
#
# eof
#
