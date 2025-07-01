#!/bin/bash
echo "Plugins"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Plugins --extra-context ./acronyms.txt ./source/cpp_src/Plugins 2>&1 | tee ./plugins.txt
echo "Done"

