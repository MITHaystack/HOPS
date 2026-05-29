################################################################################

# declare exe name
set(EXENAME "VEX2XML")

#add_subdirectory(doc)
#add_subdirectory(lib)
#add_subdirectory(src)

if(HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
