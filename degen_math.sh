#!/bin/bash
echo "Math"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Math --extra-context ./acronyms.txt ./source/cpp_src/Math 2>&1 | tee ./math.txt
echo "Done"
