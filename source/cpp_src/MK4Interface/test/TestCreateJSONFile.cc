#include <iostream>
#include <string>
#include "MHO_MK4FringeInterface.hh"

using namespace hops;

int main(int argc, char **argv) {

  // Move all the code in the type200 tests that fills the structs with 
  // data to their own files and return a pointer to the struct.
  //std::ofstream o("fringe-files.json");

  // Call interface to read fringe file
  //MHO_MK4FringeInterface* fringeReader = new MHO_MK4FringeInterface();
  MHO_MK4FringeInterface mk4FringeInterface;
  //mk4FringeInterface.ExportFringeFilesToJSON(fringeFile);
  //fringeReader->ExportFringeFilesToJSON();
  mk4FringeInterface.ExportFringeFilesToJSON();

  return 0;
}
