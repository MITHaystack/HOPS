# README for MK4Interface 

# How to run the tests

Optional: Enable tests in HOPS if running tests the first time.
```
>hops-dir/ccmake
```
- Once in the ccmake GUI use vim keys to select the test options you want to enable/disable.
- Press "ENTER" to enable/disable.
- Press "c" to configure.
- Press "g" to generate and exit.
If you do not see text referencing the "g" option at the bottom, you either haven't pressed "c" to configure
or you haven't made any changes.

1. Development dependencies (only for TestCreateJSONFile.cc): nmp, jsonlint
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



