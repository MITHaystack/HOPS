#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_afio.sh'
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

    #list of aedit header files
    declare -a source_list=(
        "adata.h"
        "afile_structure.h"
        "mk4_afio.h"
    )

    src_dir="${HOPS3_SRC_DIR}/include"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/afio/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    declare -a source_list=(
        "afile_header.c"
        "aline_id.c"
        "check_sizes.c"
        "clear_afile_structure.c"
        "clear_csumm.c"
        "clear_fsumm.c"
        "clear_rsumm.c"
        "clear_tsumm.c"
        "corelname.c"
        "fringename.c"
        "get_unique_name.c"
        "parse_csumm.c"
        "parse_fsumm.c"
        "parse_rsumm.c"
        "parse_tsumm.c"
        "read_afile.c"
        "rootname.c"
        "write_csumm.c"
        "write_fsumm.c"
        "write_rsumm.c"
        "write_tsumm.c"
    )

    src_dir="${HOPS3_SRC_DIR}/sub/afio"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/afio/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

fi

$return ${ret_val}
#
# eof
#
