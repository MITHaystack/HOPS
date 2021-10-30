#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_adump.sh'
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

    # adump headers
    declare -a source_list=( "adump.h")
    src_dir="${HOPS3_SRC_DIR}/postproc/adump"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/applications/adump/include"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    # adump (library) sources
    declare -a source_list=(
        "parse_bfields.c"
        "parse_cmdline.c"
        "parse_tfields.c"
        "strip_bline.c"
        "strip_tline.c"
        "write_header.c"
    )
    src_dir="${HOPS3_SRC_DIR}/postproc/adump"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/applications/adump/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    # adump itself
    declare -a source_list=( "adump.c")
    src_dir="${HOPS3_SRC_DIR}/postproc/adump"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/applications/adump"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))
fi

$return ${ret_val}
#
# eof
#
