hops_internal_include_directories()

set(HOPS_TEST_LIBS MHO_Message MHO_Utilities vex2json MHO_Math MHO_Containers MHO_Snapshot MHO_Operators MHO_Calibration MHO_MK4Interface MHO_Control)

set(SOURCE_BASENAMES
    #TestSingleBandDelay
    TestMultiObjectStreaming
    TestConvertMk4Visibilities
    DumpFileObjectKeys
    ConvertMk4Data
    DumpFileObjectToJSON
    ConvertFileToJSON
    ConvertVexToJSON
    TestSignalFFT
    CompareCorFiles
)

#for testing purposes only -- ROOT not intended to be a dependency
if (HOPS_USE_ROOT)
    include(FindROOT)
    find_package(ROOT REQUIRED)
    hops_internal_include_directories(../../Plugins/Root/include)
    hops_external_include_directories(${ROOT_INCLUDE_DIRS})
    add_cflag(USE_ROOT)
    list(APPEND HOPS_TEST_LIBS MHO_RootPlugin ${ROOT_LIBRARIES})
endif()

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )

################################################################################
#executables which link against additional old fourfit libraries
#(temporary measure while we try to debug/transition)
set(FFMATH_RELPATH "../../c_src/fourfit_libs/ffmath")
list(APPEND RELPATH_LIST ${FFMATH_RELPATH})

foreach(path ${RELPATH_LIST})
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

set(OLD_DEV_TEST_LIBS "ffmath")

list(APPEND OLD_DEV_SOURCE_BASENAMES
    SimpleFringeSearch
    SimpleFringeSearch_v2
)
foreach( BASENAME ${OLD_DEV_SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS} ${OLD_DEV_TEST_LIBS})
endforeach( BASENAME )
hops_install_executables( ${OLD_DEV_SOURCE_BASENAMES} )


if(HOPS_ENABLE_TEST)
   add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
