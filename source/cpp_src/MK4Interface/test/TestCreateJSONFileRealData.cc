#include <cstdlib>
#include <iostream>
#include <string>
#include "MHO_MK4FringeInterface.hh"

const int NUMBEROFFFITCHAN = 16;
const int NUMBEROFSIDEBANDSANDSBWEIGHTS = 64;
const int REFANDREMSIZE = 64;
const int AMPPHASE = 64;

using namespace hops;

int main(int argc, char **argv) {

  // Check that arguments were passed.
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " fringe/file/path data.json" << std::endl;
    return 1;
  }

  MHO_MK4FringeInterface mk4FringeInterface;
  std::string fringeFile = argv[1];
  std::string JSONfile = argv[2];

  // Do error checking on the provided fringe file path.
  std::string statFringeF = "stat "+fringeFile;
  const char* statFringeFile = statFringeF.c_str();
  if (system(statFringeFile) == 2) {
    //std::cout << "Error: Fringe file not found." << std::endl;
    std::cerr << "Error: Fringe file not found '" << fringeFile << "'" << std::endl;
    std::exit(1);
  }

  // Read the fringe file.
  std::cout << "Converting supplied fringe file to a struct..." << std::endl;
  mk4FringeInterface.ReadFringeFile(fringeFile);
  
  // Convert the fringe struct to JSON.
  const int DATASIZE = mk4FringeInterface.getType212DataSize(0);
  std::cout << "Converting struct to JSON..." << std::endl;
  mk4FringeInterface.ExportFringeFiles(JSONfile);

  std::cout << "Validating JSON..." << std::endl;
  std::string statComm = "stat "+JSONfile;
  const char* statCommand = statComm.c_str();

  // Check if JSON file exists.
  if (system(statCommand) == 2) {
    std::cout << "Error: type-200s-dump.json not found." << std::endl;
    exit(1);
  }

  // Check if JSON file was encoded correctly.
  std::string commandString = "jsonlint -q "+JSONfile+" 2> error.txt";
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
