#!/bin/sh
#
# the environment variables allows simple re-use in testing
# once installed $HOPS_JAVACLASSPATH should hold all.
#
[ -z "$CLASSPATH" ] && CLASSPATH=$HOPS_JAVACLASSPATH
[ -z "$VEX2XML4JAR" ] && VEX2XML4JAR=$CLASSPATH/java4/VEX2XML4.jar
exec java -classpath $CLASSPATH/java4 -jar $VEX2XML4JAR $@
#
# eof
#
