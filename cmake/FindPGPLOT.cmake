#very simple (probably too simple) script to
#find PGPLOT libs and includes
#TODO need to determine how PGPLOT was compiled w.r.t to fortran

FIND_LIBRARY(PGPLOT_LIBRARY
    NAMES pgplot
    HINTS /usr/lib/lib64 /usr/local /usr/local/lib /usr/local/pgplot /cluster/pgplot
    DOC "PGPLOT library.")

FIND_LIBRARY(CPGPLOT_LIBRARY
    NAMES cpgplot
    HINTS /usr/lib/lib64 /usr/local /usr/local/lib /usr/local/pgplot /cluster/pgplot
    DOC "CPGPLOT library.")

FIND_PATH(PGPLOT_INCLUDE_DIR 
    NAMES cpgplot.h
    HINTS /usr/local/pgplot /usr/local /usr/local/include /cluster/pgplot
    DOC "PGPLOT include directory.")

set(PGPLOT_LIBRARIES ${PGPLOT_LIBRARY} ${CPGPLOT_LIBRARY})
set(PGPLOT_INCLUDE_DIRS ${PGPLOT_INCLUDE_DIR})

#try to find the PNG library (this library may be used if png driver was built for PGPLOT)
#this really depends on the the details of how PGPLOT was built, of which we are not privy
find_package(PNG)
if(PNG_FOUND)
LIST(APPEND PGPLOT_LIBRARIES ${PNG_LIBRARIES})
endif(PNG_FOUND)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PGPLOT DEFAULT_MSG
                                  PGPLOT_LIBRARIES PGPLOT_INCLUDE_DIR)

mark_as_advanced(PGPLOT_INCLUDE_DIR PGPLOT_LIBRARY)
