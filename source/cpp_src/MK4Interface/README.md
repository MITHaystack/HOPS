# README for MK4Interface 

# How to run the tests

1. Dependencies: nmp, jsonlint
2. Ubuntu installation instructions:
```
>sudo apt install npm
>sudo npm install jsonlint
```
3. Compile HOPS
```
>cd hops-git/build
>cmake ../
>make && make build
```
4. Install HOPS
```
>make && make install
>source ../x86_64-4.00/bin/hops.bash
>ls ../x86_64-4.00/bin/
>TestFile
```
Where "TestFile" is the particular test file. Do not include the extension.



