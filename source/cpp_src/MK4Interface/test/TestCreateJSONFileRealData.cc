#include <cstdlib>
#include <iostream>
#include <string>
#include "MHO_MK4FringeInterface.hh"

const int NUMBEROFFFITCHAN = 16;
const int NUMBEROFSIDEBANDSANDSBWEIGHTS = 64;
const int REFANDREMSIZE = 64;
const int AMPPHASE = 64;
const int DATASIZE = 30;

using namespace hops;

int main(int argc, char **argv) {

  MHO_MK4FringeInterface mk4FringeInterface;

  // Read fringe file
  std::string fringeFile = "/home/violetp/code-projects/hops/hops-git/x86_64-4.00/data/test_data/testdata/3562/141-0002/GH.X.3.yxhoyl";
  std::cout << "Converting supplied fringe file to a struct..." << std::endl;
  mk4FringeInterface.ReadFringeFile(fringeFile);
  std::cout << "Converting struct to JSON..." << std::endl;
  mk4FringeInterface.ExportFringeFiles();

  std::cout << "Validating JSON..." << std::endl;
  std::string homeDir = getenv("HOME"); 
  std::string filePath = homeDir+"/type-200s-dump.json";
  std::string foo = "ls "+filePath;
  const char* lsCommand = foo.c_str();

  // Check if JSON file exists.
  if (system(lsCommand) == 2) {
    std::cout << "Error: type-200s-dump.json not found." << std::endl;
    exit(1);
  }

  // Check if JSON file was encoded correctly.
  std::string commandString = "jsonlint -q "+filePath+" 2> error.txt";
  const char* command = commandString.c_str();
  if (system(command) == 0){
    std::cout << "Test passed!" << std::endl;
  }
  else if (system(command) == 1) {
    std::cout << "Error: JSON error in type-200s-dump.json. See error.txt." << std::endl;
  }
  else {
    std::cout << "OS Error: Check that you have jsonlint installed via npm." << std::endl;
  }



  return 0;
}
