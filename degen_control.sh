#!/bin/bash
echo "Control"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Control --extra-context ./acronyms.txt ./source/cpp_src/Control 2>&1 | tee ./control.txt
echo "Done"

