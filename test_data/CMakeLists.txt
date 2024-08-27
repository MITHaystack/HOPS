#configure environment set-up script
find_program (BASH_PROGRAM bash REQUIRED)
find_program (TAR_PROGRAM tar REQUIRED)

# #create a place to stash downloaded test data (if manually triggered)
# file(MAKE_DIRECTORY ${DATA_INSTALL_DIR}/test_data)

# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/download_tarball.sh.in ${CMAKE_CURRENT_BINARY_DIR}/download_tarball.sh @ONLY)
# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/download_tarball2.sh.in ${CMAKE_CURRENT_BINARY_DIR}/download_tarball2.sh @ONLY)
# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/download_tarball3.sh.in ${CMAKE_CURRENT_BINARY_DIR}/download_tarball3.sh @ONLY)
# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/download_difx_tarball.sh.in ${CMAKE_CURRENT_BINARY_DIR}/download_difx_tarball.sh @ONLY)

if(BASH_PROGRAM)

    #install a script to manually download the test data
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/testdata_download_all.sh.in ${CMAKE_CURRENT_BINARY_DIR}/testdata_download_all.sh @ONLY)
    install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/testdata_download_all.sh DESTINATION ${BIN_INSTALL_DIR} )

    set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/hops4_testdata.tar.gz)

    add_test(NAME TESTDATA_COPY
      COMMAND cp ${HOPS_CACHED_TEST_DATADIR}/hops4_testdata.tar.gz ${DATA_TARBALL}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(DATA_TARBALL2 ${CMAKE_CURRENT_BINARY_DIR}/3764.tar.gz)

    add_test(NAME TESTDATA2_COPY
      COMMAND cp ${HOPS_CACHED_TEST_DATADIR}/3764.tar.gz ${DATA_TARBALL2}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    set(DATA_TARBALL3 ${CMAKE_CURRENT_BINARY_DIR}/3593.tar.gz)

    add_test(NAME TESTDATA3_COPY
      COMMAND cp ${HOPS_CACHED_TEST_DATADIR}/3593.tar.gz ${DATA_TARBALL3}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(DIFX_DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/vt9105.tar.gz)

    add_test(NAME DIFX_TESTDATA_COPY
      COMMAND cp ${HOPS_CACHED_TEST_DATADIR}/vt9105.tar.gz ${DIFX_DATA_TARBALL}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # 
    # if(HOPS_ENABLE_REMOTE_TEST_DATA)
    # 
    #     find_program (WGET_PROGRAM wget REQUIRED)
    # 
    #     add_test(NAME TESTDATA_DOWNLOAD COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/download_tarball.sh )
    #     add_test(NAME DATA2_DOWNLOAD COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/download_tarball2.sh )
    #     add_test(NAME DATA3_DOWNLOAD COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/download_tarball3.sh )
    #     add_test(NAME DIFX_TESTDATA_DOWNLOAD COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/download_difx_tarball.sh )
    # 
    #     set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/hops4_testdata.tar.gz)
    #     set(DATA_TARBALL2 ${CMAKE_CURRENT_BINARY_DIR}/3764.tar.gz)
    #     set(DATA_TARBALL3 ${CMAKE_CURRENT_BINARY_DIR}/3593.tar.gz)
    #     set(DIFX_DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/vt9105.tar.gz)
    # 
    # endif(HOPS_ENABLE_REMOTE_TEST_DATA)

    add_test(NAME TESTDATA_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    add_test(NAME TESTDATA2_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL2}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    add_test(NAME TESTDATA3_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL3}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    add_test(NAME DIFX_TESTDATA_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DIFX_DATA_TARBALL}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_tests_properties(TESTDATA_UNPACK PROPERTIES DEPENDS TESTDATA_COPY)
    set_tests_properties(TESTDATA2_UNPACK PROPERTIES DEPENDS TESTDATA2_COPY)
    set_tests_properties(TESTDATA3_UNPACK PROPERTIES DEPENDS TESTDATA3_COPY)
    set_tests_properties(DIFX_TESTDATA_UNPACK PROPERTIES DEPENDS DIFX_TESTDATA_COPY)

    # if(HOPS_ENABLE_REMOTE_TEST_DATA)
    #     set_tests_properties(TESTDATA_UNPACK PROPERTIES DEPENDS TESTDATA_DOWNLOAD)
    #     set_tests_properties(TESTDATA2_UNPACK PROPERTIES DEPENDS TESTDATA2_DOWNLOAD)
    #     set_tests_properties(TESTDATA3_UNPACK PROPERTIES DEPENDS TESTDATA3_DOWNLOAD)
    #     set_tests_properties(DIFX_TESTDATA_UNPACK PROPERTIES DEPENDS DIFX_TESTDATA_COPY)
    # endif(HOPS_ENABLE_REMOTE_TEST_DATA)

    #configure the environment script
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/chk_cmake_env.sh ${CMAKE_CURRENT_BINARY_DIR}/chk_env.sh @ONLY)

    add_test(NAME HOPS_ENV_SETUP
        COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/chk_env.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

endif(BASH_PROGRAM)
