#include <iostream>
#include <string>
#include "MHO_MK4FringeInterface.hh"

using namespace hops;

int main(int argc, char **argv) {

  // Call all the type200 tests here
  //std::ofstream o("fringe-files.json");

  // Call interface to read fringe file
  //MHO_MK4FringeInterface* fringeReader = new MHO_MK4FringeInterface();
  MHO_MK4FringeInterface mk4FringeInterface;
  //mk4FringeInterface.ExportFringeFilesToJSON(fringeFile);
  //fringeReader->ExportFringeFilesToJSON();
  mk4FringeInterface.ExportFringeFilesToJSON();

  return 0;
}
