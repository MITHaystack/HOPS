#!/bin/sh
#
exec java -classpath $HOPS_JAVACLASSPATH/lib3 \
    -jar $HOPS_JAVACLASSPATH/lib3/VEX2XML.jar $@
#
# eof
#
