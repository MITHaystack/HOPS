#configure environment set-up script
find_program (BASH_PROGRAM bash)
find_program (TAR_PROGRAM tar)
find_program (WGET_PROGRAM wget)

if(HOPS_ENABLE_TEST)
    add_subdirectory(hops3-ff-tests)
endif(HOPS_ENABLE_TEST)

# if(HOPS_ENABLE_TEST)
#    add_subdirectory(hops3-ae-tests)
# endif(HOPS_ENABLE_TEST)

# eof
