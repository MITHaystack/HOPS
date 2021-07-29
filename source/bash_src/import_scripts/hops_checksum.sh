#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

ret_val=0

echo HOPS4_SRC_DIR="$HOPS4_SRC_DIR"
echo HOPS3_SRC_DIR="$HOPS3_SRC_DIR"

if [ "$HOPS4_SRC_DIR" != "" ] && [ "$HOPS3_SRC_DIR" != "" ]; then
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_afio.sh --checksum-only)
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_dfio.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_mk4util.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_vex.sh --checksum-only)
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffcontrol.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffcore.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffio.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffmath.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffplot.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffsearch.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_fourfit.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_alist.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi
    (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_aedit.sh --checksum-only )
    tmp_val=$?
    if [ "$tmp_val" -ne "0" ]
    then
        ret_val=1
    fi

else

    echo "Please define the variables HOPS4_SRC_DIR and HOPS3_SRC_DIR before running this script."
    ret_val=2

fi

return ${ret_val}
