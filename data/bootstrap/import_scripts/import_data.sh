#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_data.sh'
me=`basename $0 2>&-` || me=sh
[ "$me" = "$part" ] && return=exit || return=return

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

    # snratio itself
    declare -a source_list=( "legacy_unpack.sh")
    src_dir="${HOPS3_SRC_DIR}/data"
    dest_dir="${HOPS4_SRC_DIR}/data/bootstrap"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))
fi

$return ${ret_val}
#
# eof
#
