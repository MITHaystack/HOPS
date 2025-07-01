#!/bin/bash
echo "Message"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Message --extra-context ./acronyms.txt ./source/cpp_src/Message 2>&1 | tee ./message.txt
echo "Done"

