#!/bin/bash
echo "Operators"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Operators --extra-context ./acronyms.txt ./source/cpp_src/Operators 2>&1 | tee operator.txt
echo "Done"
