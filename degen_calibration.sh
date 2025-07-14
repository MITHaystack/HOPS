#!/bin/bash
echo "Calibration"
degen --llm-enhance --llm-endpoint http://localhost:11434/v1 --llm-model mistral-nemo:12b --collect-context ./source/cpp_src/Calibration --extra-context ./acronyms.txt ./source/cpp_src/Calibration 2>&1 | tee ./calibration.txt
echo "Done"

