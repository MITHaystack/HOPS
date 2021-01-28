
set(MK4UTIL_RELPATH "../../../c_src/mk4util/include")
set(DFIO_RELPATH "../../../c_src/dfio/include")
set(VEX_RELPATH "../../../c_src/vex/include")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Containers/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Math/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Math MHO_MK4Interface mk4util dfio vex )

set(SOURCE_BASENAMES
    TestMK4VexInterface
    TestMK4Import
)

#for testing purposes only -- not intended to be a dependency
if (HOPS_USE_ROOT)
    include(FindROOT)
    find_package(ROOT REQUIRED)
    hops_external_include_directories(${ROOT_INCLUDE_DIRS})
    add_cflag(USE_ROOT)
    list(APPEND HOPS_TEST_LIBS ${ROOT_LIBRARIES})
endif()

if(HOPS_USE_FFTW3)
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
    list(APPEND HOPS_TEST_LIBS ${FFTW_LIBRARIES})
endif (HOPS_USE_FFTW3)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
