#!/bin/bash
find . -name "*.cc" -o -name "*.hh" | xargs clang-format --style=file -i
