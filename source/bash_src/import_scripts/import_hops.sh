#!/bin/bash

#check if we were passed the flag --checksum-only, if so, we only need to
#compare the files, and return 0 (1) if they are the same (different)

CHKSUM=0
if [ "$1" == "--checksum-only" ]; then
	CHKSUM=1
fi

ret_val=0

if [ "$HOPS4_SRC_DIR" != "" ] && [ "$HOPS3_SRC_DIR" != "" ]; then

    echo HOPS4_SRC_DIR="$HOPS4_SRC_DIR"
    echo HOPS3_SRC_DIR="$HOPS3_SRC_DIR"


    if [ "${CHKSUM}" -eq "0" ]
    then
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_afio.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_dfio.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_mk4util.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_vex.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/ffcontrol/import_ffcontrol.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/ffcore/import_ffcore.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/ffio/import_ffio.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/ffmath/import_ffmath.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/ffplot/import_ffplot.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/ffsearch/import_ffsearch.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/fourfit/import_fourfit.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/alist/import_alist.sh
        source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/aedit/import_aedit.sh
    else
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_afio.sh --checksum-only)
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_afio.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_dfio.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_mk4util.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_vex.sh --checksum-only)
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffcontrol.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffcore.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffio.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffmath.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffplot.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi
        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_ffsearch.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi

        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_fourfit.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi

        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_alist.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi

        (source ${HOPS4_SRC_DIR}/source/bash_src/import_scripts/import_aedit.sh --checksum-only )
        tmp_val=$?
        if [ $tmp_val ]
        then
            ret_val=1
        fi

    fi

else

    echo "Please define the variables HOPS4_SRC_DIR and HOPS3_SRC_DIR before running this script."
    ret_val=2

fi

return ${ret_val}
