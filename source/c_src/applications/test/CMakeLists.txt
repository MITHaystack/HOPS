#configure environment set-up script
find_program (BASH_PROGRAM bash)
find_program (TAR_PROGRAM tar)
find_program (WGET_PROGRAM wget)

option( HOPS_ENABLE_REMOTE_TEST_DATA "Download remote data for developer tests." OFF)

# if(BASH_PROGRAM)
# 
#     set(TEST_SCRIPT_BASENAMES
#         chk_2611
#         chk_2836
#         chk_2843
#         chk_3365
#         chk_3372
#         chk_3413
#         chk_3571
#         chk_3756
#     )
#     foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
#         configure_file(
#             ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh
#             ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
#         add_test(NAME ${BASENAME} COMMAND
#             ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
#         set_tests_properties(${BASENAME} PROPERTIES ENVIRONMENT "srcdir=${CMAKE_CURRENT_SOURCE_DIR};abs_top_srcdir=${CMAKE_SOURCE_DIR};abs_top_builddir=${CMAKE_BINARY_DIR}")
#     endforeach( BASENAME )
# 
# endif(BASH_PROGRAM)

# temporary access to the native tests

if(HOPS_ENABLE_TEST)
    add_subdirectory(hops3-ff-tests)
endif(HOPS_ENABLE_TEST)

# if(HOPS_ENABLE_TEST)
#    add_subdirectory(hops3-ae-tests)
# endif(HOPS_ENABLE_TEST)

# eof

