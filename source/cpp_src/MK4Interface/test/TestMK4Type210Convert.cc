#include "MHO_MK4Type210Converter.hh"
#include <iostream>
#include <stdlib.h>
#include <string>

const int AMPPHASE = 64;

using namespace hops;

int main(int argc, char **argv) {
  struct type_210 my210;

  // create and fill in a type_210 struct with some dummy data
  strcpy(my210.record_id, "202");
  strcpy(my210.version_no, "2");
  strcpy(my210.unused1, "unused...");
  for (int i = 0; i < AMPPHASE; i++) {
    my210.amp_phas[i].ampl = 22.22;
    my210.amp_phas[i].phase = 22.22;
  }

  json obj = convertToJSON(my210);
  std::cout << obj.dump(2) << std::endl;

  return 0;
}
