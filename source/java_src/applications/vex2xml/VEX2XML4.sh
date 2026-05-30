#!/bin/sh
#
exec java -classpath $HOPS_JAVACLASSPATH/lib4 \
    -jar $HOPS_JAVACLASSPATH/lib4/VEX2XML.jar $@
#
# eof
#
