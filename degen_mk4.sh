#!/bin/bash
echo "MK4"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/MK4Interface --extra-context ./acronyms.txt ./source/cpp_src/MK4Interface 2>&1 | tee mk4.txt
echo "Done"

