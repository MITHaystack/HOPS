#paths to look in for libgfortran
#TODO make this script more robust

enable_language(Fortran)
get_filename_component (Fortran_COMPILER_NAME ${CMAKE_Fortran_COMPILER} NAME)

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


set(GFORTRAN_LIBRARIES ${GFORTRAN_LIB})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GFORTRAN DEFAULT_MSG GFORTRAN_LIBRARIES)

mark_as_advanced(PGPLOT_INCLUDE_DIR GFORTRAN_LIBRARIES)
