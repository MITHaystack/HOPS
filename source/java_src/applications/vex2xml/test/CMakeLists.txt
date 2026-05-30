################################################################################
#
# the test is to run through all the vex and verify against captured xml
# using some of the logic from hops3-ff-tests
if(HOPS_ENABLE_TEST)

    set(EXPN_NAMES "")
    list(APPEND EXPN_NAMES
        a25gl b23gl bm452c c151a c151b c152a c152b
        c22gl d21us e16b08 g26us gbc1 new27)

    set(cp ${CMAKE_CURRENT_SOURCE_DIR}/../lib/java3)
    set(vj ${CMAKE_CURRENT_BINARY_DIR}/../src/VEX2XML3.jar)

    foreach( EXPN_NAME ${EXPN_NAMES} )
        add_test(
            NAME ${EXPN_NAME}_v2v_test
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../VEX2XML3.sh
                -in ${CMAKE_CURRENT_SOURCE_DIR}/vex/${EXPN_NAME}.vex
                -out ${CMAKE_CURRENT_BINARY_DIR}/${EXPN_NAME}.xml &&
            cmp ${CMAKE_CURRENT_BINARY_DIR}/${EXPN_NAME}.xml
                ${CMAKE_CURRENT_SOURCE_DIR}/xml/${EXPN_NAME}.xml)
        set_tests_properties(${EXPN_NAME}_v2v_test PROPERTIES ENVIRONMENT
            "CLASSPATH=${cp};VEX2XML3JAR=${vj}")
        
    endforeach( EXPN_NAME )

endif(HOPS_ENABLE_TEST)


