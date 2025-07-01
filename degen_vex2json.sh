#!/bin/bash
echo "Vex2JSON"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Vex2JSON --extra-context ./acronyms.txt ./source/cpp_src/Vex2JSON 2>&1 | tee ./vex2json.txt
echo "Done"

