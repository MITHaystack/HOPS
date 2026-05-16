#!/bin/bash
#
# Script to verify GSL minimizer example output.
#
[ -f 'exminimizer.log' ] || {
    echo No execution of exminimizer present.  Skipping out.
    exit 77
}
[ -z "$srcdir" ] && {
    echo no srcdir provided, this is a serious issue
    exit 99
}
ref="$srcdir/exminimizer.output"
[ -f "$ref" ] || { echo Missing reference $ref ; exit 99; }

grep -v PASS exminimizer.log | cmp - "$ref" && {
    echo compiled output agrees with source output
    exit 0
} || {
    echo output differs from reference
    exit 1
}
#
# eof vim:nospell
#
