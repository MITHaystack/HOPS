#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_ffio.sh'
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

    # fourfit files that write mk4 fringe files
    declare -a source_list=(
    "fill_200.c"
    "fill_201.c"
    "fill_202.c"
    "fill_203.c"
    "fill_204.c"
    "fill_205.c"
    "fill_206.c"
    "fill_207.c"
    "fill_208.c"
    "fill_210.c"
    "fill_212.c"
    "fill_222.c"
    "fill_230.c"
    "fill_fringe_info.c"
    )
    
    src_dir="${HOPS3_SRC_DIR}/postproc/fourfit"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/fourfit_libs/ffio/src"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))
fi

$return ${ret_val}
#
# eof
#
