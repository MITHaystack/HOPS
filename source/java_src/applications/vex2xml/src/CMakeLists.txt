################################################################################
# build the VEX2XML.jar makes use of the antlr grammar; in HOPS3:
#   antlr = antlr-3.5.2-complete.jar
# or
#   antlr = antlr-4.8-complete.jar
#
# java -jar $(antlr) vexGrammar.g
# making:
#   vexGrammarParser.java vexGrammar.tokens vexGrammarLexer.java
# and it is not clear if 4.8 was actually validated.
# here we have completely separated the code into parallel versions
#
# compilation was then (-d. says put the generated class files .):
#   javac $(srcdir)/src/VEX2XML.java -cp ".:$(srcdir)/@ANTLR4LIB@/*" -d .
# and jar produced by:
#   cp -p $(antlr) $(cmmns) .
#   jar -cvfm ./VEX2XML.jar manifest.txt *.class > jarring.txt 2>&1
#
# and automake had cleanup (rm -f vexGrammar[LP]*)
#
include(UseJava)

set(JLIB3 ${CMAKE_CURRENT_SOURCE_DIR}/../lib/java3)
set(JLIB4 ${CMAKE_CURRENT_SOURCE_DIR}/../lib/java4)
set(JSRC ${CMAKE_CURRENT_SOURCE_DIR})

if(HOPS_VEX2XML3)
    add_custom_command(OUTPUT vexGrammar3Parser.java vexGrammar3.tokens vexGrammar3Lexer.java
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ${JLIB3}/antlr-3.5.2-complete.jar ${JSRC}/vexGrammar3.g
        COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/vexGrammar3Parser.java ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/vexGrammar3Lexer.java ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${JSRC}/vexGrammar3.g ${JSRC}/manifest3.txt)
    add_jar(VEX2XML3
        SOURCES VEX2XML3.java vexGrammar3Parser.java vexGrammar3.tokens vexGrammar3Lexer.java
        INCLUDE_JARS ${JLIB3}/antlr-3.5.2-complete.jar ${JLIB3}/commons-cli-1.2.jar)
endif(HOPS_VEX2XML3)
if(HOPS_VEX2XML4)
    add_custom_command(OUTPUT vexGrammar4Parser.java vexGrammar4.tokens vexGrammar4Lexer.java
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ${JLIB4}/java4/antlr-4.8-complete.jar ${JSRC}/vexGrammar4.g
        COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/vexGrammar4Parser.java ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/vexGrammar4Lexer.java ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${JSRC}/vexGrammar4.g ${JSRC}/manifest4.txt)
    add_jar(VEX2XML4
        SOURCES VEX2XML4.java vexGrammar4Parser.java vexGrammar4.tokens vexGrammar4Lexer.java
        INCLUDE_JARS ${JLIB4}/antlr-4.8-complete.jar ${JLIB4}/commons-cli-1.4.jar)
endif(HOPS_VEX2XML4)

