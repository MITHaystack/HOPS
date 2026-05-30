#!/bin/sh
#
# the environment variables allows simple re-use in testing
# once installed $HOPS_JAVACLASSPATH should hold all.
#
[ -z "$CLASSPATH" ] && CLASSPATH=$HOPS_JAVACLASSPATH
[ -z "$VEX2XML3JAR" ] && VEX2XMLJAR=$CLASSPATH/java3/VEX2XML3.jar
exec java -classpath $CLASSPATH/java3 -jar $VEX2XML3JAR $@
#
# eof
#
