#include <iostream>
#include <string>

#include "MHO_MK4Type201Converter.hh"

using namespace hops;

int main(int argc, char **argv) {

  // create and fill in a type_200 struct with some dummy data
  struct type_201 my201;

  my201.record_id[0] = '2';
  my201.record_id[1] = '0';
  my201.record_id[2] = '1';
  my201.version_no[0] = '0';
  my201.version_no[1] = '0';
  strncpy(my201.unused1, "foo", sizeof(my201.unused1));
  strncpy(my201.source, "WYQXVxN4Ci6FAU7by7wQLPOnhGFlPGSx", sizeof(my201.source));
  my201.coord.ra_hrs = 1;
  my201.coord.ra_mins = 1;
  my201.coord.ra_secs = 1;
  my201.coord.dec_degs = 1;
  my201.coord.dec_mins = 1;
  my201.coord.dec_secs = 1;
  my201.epoch = 2;
  for (int i = 0; i < 4; i++) {
    my201.pulsar_phase[i] = 1;
  }
  strncpy(my201.unused2, "bar", sizeof(my201.unused2));
  my201.ra_rate = 2;
  my201.dec_rate = 2;
  my201.pulsar_epoch = 2;
  my201.dispersion = 2;

  mho_json obj = convertToJSON(my201);

  std::cout << obj.dump(2) << std::endl;

  return 0;
}
