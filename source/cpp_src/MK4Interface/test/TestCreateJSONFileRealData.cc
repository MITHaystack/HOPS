#include <cstdlib>
#include <iostream>
#include <string>
#include "MHO_MK4FringeInterface.hh"

const int NUMBEROFFFITCHAN = 16;
const int NUMBEROFSIDEBANDSANDSBWEIGHTS = 64;
const int REFANDREMSIZE = 64;
const int AMPPHASE = 64;
const int DATASIZE = 1;

using namespace hops;

int main(int argc, char **argv) {

  MHO_MK4FringeInterface mk4FringeInterface;

  // Read fringe file
  std::string fringeFile = "/home/violetp/code-projects/hops/hops-git/x86_64-4.00/data/test_data/testdata/3562/141-0002/GH.X.3.yxhoyl";
  mk4FringeInterface.ReadFringeFile(fringeFile);
  //mk4FringeInterface.ExportFringeFiles();

  // convert to JSON (data dumped automatically)
  
  // Add the code from TestCreatJSONFile.cc that checks if the JSON file was dumped 
  // and verifies that it is valid JSON.

  return 0;
}
