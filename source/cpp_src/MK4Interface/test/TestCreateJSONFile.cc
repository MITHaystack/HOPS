#include <iostream>
#include <string>
#include "MHO_MK4Type200Converter.hh"
#include "MHO_MK4Type201Converter.hh"
#include "MHO_MK4Type202Converter.hh"
#include "MHO_MK4Type203Converter.hh"
#include "MHO_MK4Type204Converter.hh"
#include "MHO_MK4Type205Converter.hh"
#include "MHO_MK4Type206Converter.hh"
#include "MHO_MK4Type207Converter.hh"
#include "MHO_MK4Type208Converter.hh"
#include "MHO_MK4Type210Converter.hh"
#include "MHO_MK4Type212Converter.hh"
#include "MHO_MK4FringeInterface.hh"

using namespace hops;

int main(int argc, char **argv) {

  // Call all the type200 tests here

  // Call interface to read fringe file
  MHO_MK4FringeInterface* fringeReader = new MHO_MK4FringeInterface();
  //MHO_MK4FringeInterface mk4FringeInterface;
  //mk4FringeInterface.ExportFringeFilesToJSON(fringeFile);
  fringeReader->ExportFringeStructsToJSON();

  return 0;
}
