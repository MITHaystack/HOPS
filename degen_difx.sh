#!/bin/bash
echo "DiFX"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/DiFXInterface --extra-context ./acronyms.txt ./source/cpp_src/DiFXInterface 2>&1 | tee ./difx.txt
echo "Done"

