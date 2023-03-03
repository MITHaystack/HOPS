#configure environment set-up script
find_program (BASH_PROGRAM bash)
find_program (TAR_PROGRAM tar)
find_program (WGET_PROGRAM wget)

if(BASH_PROGRAM)

    if(HOPS_ENABLE_REMOTE_TEST_DATA)

        set(REMOTE_DATA "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/vt9105.tar.gz")
        set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/vt9105.tar.gz)
        set(DATADIR ${CMAKE_CURRENT_BINARY_DIR}/vt9105)
        set(CMAKE_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR})

        add_test(NAME DIFX_TESTDATA_DOWNLOAD
          COMMAND ${WGET_PROGRAM} -q ${REMOTE_DATA} -O ${DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        add_test(NAME DIFX_TESTDATA_UNPACK
            COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        set_tests_properties(DIFX_TESTDATA_UNPACK PROPERTIES DEPENDS DIFX_TESTDATA_DOWNLOAD)

   endif(HOPS_ENABLE_REMOTE_TEST_DATA)

endif(BASH_PROGRAM)
