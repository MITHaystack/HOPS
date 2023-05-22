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

using namespace hops;

int main(int argc, char **argv) {

  // Open file
  std::string fringeFile = filename;
  // Call interface to read fringe file
  MHO_MK4FringeInterface* fringeReader = new MHO_MK4FringeInterface();
  // Dump it in to fFringe field of that interface
  fringeReader.ReadFringeFile(&filename);
  mk4_fringe fringeReader.fFringe

  // Dump to file
  //std::ofstream outputFile(type200-dump.js);
  //outputFile << std::setw(4) << my200s << std::endl;
  //outputFile.close()

  return 0;
}
