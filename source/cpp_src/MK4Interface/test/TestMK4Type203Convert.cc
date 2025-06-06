#include <iostream>
#include <stdlib.h>
#include <string>

#include "MHO_MK4Type203Converter.hh"

using namespace hops;

int main(int argc, char **argv) {
  srand(time(NULL));

  struct type_203 my203;

  // create and fill in a type_203 struct with some dummy data
  strncpy(my203.record_id, "202000000", sizeof(my203.record_id));
  strncpy(my203.version_no, "000000", sizeof(my203.version_no));

  for (int i = 0; i < 512; i++) {
    // initialize all channels except the last one
    if (i < 511) {
      my203.channels[i].index = i;
      my203.channels[i].sample_rate = rand() % 20 + 1;
      my203.channels[i].refsb = 'a' + rand() % 26;
      my203.channels[i].remsb = 'a' + rand() % 26;
      my203.channels[i].rempol = 'a' + rand() % 26;
      my203.channels[i].ref_freq = rand() % 20 + 1 + .5;
      my203.channels[i].rem_freq = rand() % 20 + 1 + .5;
      strncpy(my203.channels[i].ref_chan_id, "foobarfoo", sizeof(my203.channels[i].ref_chan_id));
      strncpy(my203.channels[i].rem_chan_id, "barfoo", sizeof(my203.channels[i].rem_chan_id));
    }
  }

  mho_json obj = convertToJSON(my203);
  std::cout << obj.dump(2) << std::endl;

  return 0;
}
