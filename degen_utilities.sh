#!/bin/bash
echo "Utilities"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Utilities --extra-context ./acronyms.txt ./source/cpp_src/Utilities 2>&1 | tee ./utilities.txt
echo "Done"
