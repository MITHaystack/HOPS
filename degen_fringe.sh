#!/bin/bash
echo "Fringe"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Fringe --extra-context ./acronyms.txt ./source/cpp_src/Fringe 2>&1 | tee ./fringe.txt
echo "Done"

