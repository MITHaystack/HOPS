#!/bin/bash
echo "Initialization"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Initialization --extra-context ./acronyms.txt ./source/cpp_src/Initialization 2>&1 | tee ./initialization.txt
echo "Done"

