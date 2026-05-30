################################################################################
# build the VEX2XML.jar; see ../doc/automakefile.txt for how it was autobuilt.
#
# this appears to be totally broken:
#include(UseJava)
# it wants to work in the source dir
#message(STATUS "Java     is ${Java_JAVAC_EXECUTABLE}")
#message(STATUS "Javac    is ${Java_JAVA_EXECUTABLE}")
#message(STATUS "Java Jar is ${Java_JAR_EXECUTABLE}")

set(JLIB3 ${CMAKE_CURRENT_SOURCE_DIR}/../lib/java3)
set(JLIB4 ${CMAKE_CURRENT_SOURCE_DIR}/../lib/java4)
set(JSRC ${CMAKE_CURRENT_SOURCE_DIR})
set(JCBD ${CMAKE_CURRENT_BINARY_DIR})

if(HOPS_VEX2XML3)
    add_custom_command(
        OUTPUT vexGrammar3Parser.java vexGrammar3.tokens vexGrammar3Lexer.java
            antlr-3.5.2-complete.jar commons-cli-1.2.jar manifest3.txt VEX2XML3.java
        COMMAND cp -p ${JLIB3}/antlr-3.5.2-complete.jar .
        COMMAND cp -p ${JLIB3}/commons-cli-1.2.jar .
        COMMAND cp -p ${JSRC}/manifest3.txt .
        COMMAND cp -p ${JSRC}/vexGrammar3.g .
        COMMAND cp -p ${JSRC}/VEX2XML3.java .
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ./antlr-3.5.2-complete.jar vexGrammar3.g
        DEPENDS ${JSRC}/vexGrammar3.g ${JSRC}/manifest3.txt ${JSRC}/VEX2XML3.java
            ${JLIB3}/antlr-3.5.2-complete.jar ${JLIB3}/commons-cli-1.2.jar)
    add_custom_command(
        OUTPUT VEX2XML3.jar
        COMMAND ${Java_JAVAC_EXECUTABLE} VEX2XML3.java -cp ".:./antlr-3.5.2-complete.jar:./commons-cli-1.2.jar" -d .
        COMMAND ${Java_JAR_EXECUTABLE} -cvfm ./VEX2XML3.jar manifest3.txt *.class > jarring.txt 2>&1
        DEPENDS vexGrammar3Parser.java vexGrammar3.tokens vexGrammar3Lexer.java
            antlr-3.5.2-complete.jar commons-cli-1.2.jar manifest3.txt)
    add_custom_target(v2x3 ALL touch v2x3 COMMAND rm -f vexGrammar3*class DEPENDS VEX2XML3.java VEX2XML3.jar)
    install(FILES ${JCBD}/VEX2XML3.jar DESTINATION ${JAVA_INSTALL_DIR}/java3)
endif(HOPS_VEX2XML3)
if(HOPS_VEX2XML4)
    add_custom_command(
        OUTPUT vexGrammar4Parser.java vexGrammar4.tokens vexGrammar4Lexer.java
            antlr-4.5.2-complete.jar commons-cli-1.2.jar manifest4.txt VEX2XML4.java
        COMMAND cp -p ${JLIB4}/antlr-4.8-complete.jar .
        COMMAND cp -p ${JLIB4}/commons-cli-1.4.jar .
        COMMAND cp -p ${JSRC}/manifest4.txt .
        COMMAND cp -p ${JSRC}/vexGrammar4.g .
        COMMAND cp -p ${JSRC}/VEX2XML4.java .
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ./antlr-4.8-complete.jar vexGrammar4.g
        DEPENDS ${JSRC}/vexGrammar4.g ${JSRC}/manifest4.txt ${JSRC}/VEX2XML4.java
            ${JLIB4}/antlr-4.8-complete.jar ${JLIB4}/commons-cli-1.4.jar)
    add_custom_command(
        OUTPUT VEX2XML4.jar
        COMMAND ${Java_JAVAC_EXECUTABLE} VEX2XML4.java -cp ".:./antlr-4.8-complete.jar:./commons-cli-1.4.jar" -d .
        COMMAND ${Java_JAR_EXECUTABLE} -cvfm ./VEX2XML4.jar manifest4.txt *.class > jarring.txt 2>&1
        DEPENDS vexGrammar4Parser.java vexGrammar4.tokens vexGrammar4Lexer.java
            antlr-4.5.2-complete.jar commons-cli-1.2.jar manifest4.txt)
    add_custom_target(v2x4 ALL touch v2x4 DEPENDS VEX2XML4.java VEX2XML4.jar)
    install(FILES ${JCBD}/VEX2XML4.jar DESTINATION ${JAVA_INSTALL_DIR}/java4)
endif(HOPS_VEX2XML4)

