#include "MHO_MK4Type212Converter.hh"
#include <iostream>
#include <stdlib.h>
#include <string>

const int DATASIZE = 1;

using namespace hops;

int main(int argc, char **argv) {
  struct type_212 my212;

  // create and fill in a type_212 struct with some dummy data
  strncpy(my212.record_id, "202", sizeof(my212.record_id));
  strncpy(my212.version_no, "2", sizeof(my212.version_no));
  my212.unused = 'u';
  my212.nap = 2;
  my212.first_ap = 2;
  my212.channel = 25;
  my212.sbd_chan = 2;
  strncpy(my212.unused2, "unused2...", sizeof(my212.unused2));
  for (int i = 0; i < DATASIZE; i++) {
    my212.data[i].amp = 22.22;
    my212.data[i].phase = 22.22;
    my212.data[i].weight = 22.22;
  }

  json obj = convertToJSON(my212);
  std::cout << obj.dump(2) << std::endl;

  return 0;
}
