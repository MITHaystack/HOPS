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

  // Read the fringe file.
  std::cout << "Converting supplied fringe file to a struct..." << std::endl;
  mk4FringeInterface.ReadFringeFile(fringeFile);
  
  // Convert the fringe struct to JSON.
  const int DATASIZE = mk4FringeInterface.getType212DataSize(0);
  std::cout << "Converting struct to JSON..." << std::endl;
  mk4FringeInterface.ExportFringeFiles(JSONfile);

  std::cout << "Don't forget to validate the JSON with jsonlint or another such similar package. i.e.: " << "jsonlint -q " << JSONfile << std::endl;

  return 0;
}
