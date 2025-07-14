#!/bin/bash
echo "Containers"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Containers --extra-context ./acronyms.txt ./source/cpp_src/Containers 2>&1 | tee ./containers.txt
echo "Done"

