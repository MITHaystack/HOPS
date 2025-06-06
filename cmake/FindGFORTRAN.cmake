#paths to look in for libgfortran
#TODO make this script more robust
set(GFORTRAN_PATHS /usr/lib
                   /usr/lib64
                   /usr/local/lib
                   /usr/local/lib64
                   /usr/lib/openmpi/lib
                   /usr/lib/gcc/x86_64-linux-gnu/4.8
                   /usr/lib/gcc/x86_64-linux-gnu/4.9
                   /usr/lib/gcc/x86_64-linux-gnu/5
                   /usr/lib/gcc/x86_64-linux-gnu/6
                   /usr/lib/gcc/x86_64-linux-gnu/7
                   /usr/lib/gcc/x86_64-linux-gnu/8
                   /usr/lib/gcc/x86_64-linux-gnu/9
                   /lib/x86_64-linux-gnu
                   /usr/lib/x86_64-linux-gnu
                   /usr/lib/openblas-base
                   /usr/lib64/openblas-base
                   /usr/local/lib/gcc/4.8
                   /usr/local/lib/gcc/4.9
                   /usr/local/lib/gcc/5
                   /usr/local/lib/gcc/6)


if(NOT GFORTRAN_LIB)
    FIND_LIBRARY(GFORTRAN_LIB
        NAMES gfortran
        HINTS ${GFORTRAN_PATHS}
        DOC "gfortran standard library."
    )
endif()

#what we need is the gfortran version which pgplot is linked against
#look for a versioned copy of the library .so.4
#on DEMI -- this is libgfortran.so.4 (so check this first)
if(NOT GFORTRAN_LIB)
    FIND_LIBRARY(GFORTRAN_LIB
        NAMES libgfortran.so.4
        HINTS ${GFORTRAN_PATHS}
        DOC "gfortran standard library."
    )
endif()

#look for a versioned copy of the library .so.5
if(NOT GFORTRAN_LIB)
    FIND_LIBRARY(GFORTRAN_LIB
        NAMES libgfortran.so.5
        HINTS ${GFORTRAN_PATHS}
        DOC "gfortran standard library."
    )
endif()

#look for a versioned copy of the library .so.3
if(NOT GFORTRAN_LIB)
    FIND_LIBRARY(GFORTRAN_LIB
        NAMES libgfortran.so.3
        HINTS ${GFORTRAN_PATHS}
        DOC "gfortran standard library."
    )
endif()

set(GFORTRAN_LIBRARIES ${GFORTRAN_LIB})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GFORTRAN DEFAULT_MSG GFORTRAN_LIBRARIES)

mark_as_advanced(GFORTRAN_INCLUDE_DIR GFORTRAN_LIBRARIES)
