#!/bin/sh

#simple script for developers to update the canned copy of the git submodules (date, json, pybind11)
#must be run from the <hops-git>/extern directory
#make sure to clone the submodules first with (from the root repo directory):
# git submodule update --init --recursive

# cd ./submodules/date
# git archive --format=tar.gz HEAD > ../../date.tar.gz
# cd ../../
# mkdir -p date 
# cd ./date
# tar -xzvf ../date.tar.gz 
# cd ../
# rm ./date.tar.gz

# cd ./submodules/pybind11
# git archive --format=tar.gz HEAD > ../../pybind11.tar.gz
# cd ../../
# mkdir -p ./pybind11
# cd ./pybind11
# tar -xzvf ../pybind11.tar.gz 
# cd ../
# rm ./pybind11.tar.gz

# cd ./submodules/matplotplusplus
# git archive --format=tar.gz HEAD > ../../matplotplusplus.tar.gz
# cd ../../
# mkdir -p ./matplotplusplus
# cd ./matplotplusplus
# tar -xzvf ../matplotplusplus.tar.gz 
# cd ../
# rm ./matplotplusplus.tar.gz

# cp ./submodules/json/single_include/nlohmann/json.hpp ./json/include/nlohmann/json.hpp
# 
# cp ./submodules/pybind11_json/include/pybind11_json/pybind11_json.hpp ./json/include/pybind11_json/pybind11_json.hpp
# 
