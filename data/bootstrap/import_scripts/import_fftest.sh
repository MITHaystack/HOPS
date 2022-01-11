#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

# allow this to be sourced from ./import_hops.sh or executed standalone
part='import_fourfit.h'
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

    #copy fourfit tests for comparison
    declare -a source_list=(
    chk_adump.sh
    chk_aedit.sh
    chk_alist.sh
    chk_avefix.sh
    chk_average.sh
    chk_baselines.sh
    chk_cofit.sh
    chk_env.sh
    chk_ff_2836.sh
    chk_ff_2843.sh
    chk_ff_3372.sh
    chk_ff_3413.sh
    chk_ff_3571.sh
    chk_ff_3756.sh
    chk_ff_3772pb.sh
    chk_ff_display.sh
    chk_ff_dump.sh
    chk_ff_ps2pdf.sh
    chk_flagging.sh
    chk_fourmer.sh
    chk_fringex.sh
    chk_frmrsrch.sh
    chk_hdlinks.sh
    chk_min_weight.sh
    chk_notches.sh
    chk_notches_xf.sh
    chk_passband.sh
    chk_ps2pdf.sh
    chk_search.sh
    )

    src_dir="${HOPS3_SRC_DIR}/data/ff_testdata"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/applications/test/hops3-ff-tests"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))

    #copy aedit tests for comparison
    declare -a source_list=( chk_fsumm.sh chk_env.sh )

    src_dir="${HOPS3_SRC_DIR}/data/ae_testdata"
    dest_dir="${HOPS4_SRC_DIR}/source/c_src/applications/test/hops3-ae-tests"
    source $bsi/compare_src_dest.sh
    ret_val=$(($ret_val + $?))
fi

$return ${ret_val}
#
# eof
#
