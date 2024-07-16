#!/bin/bash

find . -type f -name "*.py.in" -print0 | xargs -0 sed -i 's/hopstestb/hops_test/g'
