#include <iostream>
#include <string>

#include "MHO_MK4Type200Converter.hh"

using namespace hops;

int main(int argc, char **argv) {

  // create and fill in a type_200 struct with some dummy data
  struct type_200 my200;

  my200.record_id[0] = '2';
  my200.record_id[1] = '0';
  my200.record_id[2] = '0';
  my200.version_no[0] = '0';
  my200.version_no[1] = '0';
  strncpy(my200.unused1, "fooo", sizeof(my200.unused1));
  for (int i = 0; i < 10; i++) {
    my200.software_rev[i] =
        1; // i have no idea what this field is actually used for
  }
  my200.expt_no = 1234;
  strncpy(my200.exper_name, "WYQXVxN4Ci6FAU7by7wQLPOnhGFlPGSx", sizeof(my200.exper_name));
  strncpy(my200.scan_name, "NQxu5N642mOyC49l", sizeof(my200.scan_name));
  strncpy(my200.correlator, "baab", sizeof(my200.correlator));
  my200.scantime.year = 2022;
  my200.scantime.day = 22;
  my200.scantime.hour = 22;
  my200.scantime.minute = 22;
  my200.scantime.second = 22;
  my200.start_offset = 0;
  my200.stop_offset = 1;
  my200.corr_date.year = 2022;
  my200.corr_date.day = 22;
  my200.corr_date.hour = 22;
  my200.corr_date.minute = 22;
  my200.corr_date.second = 22;
  my200.fourfit_date.year = 2022;
  my200.fourfit_date.day = 22;
  my200.fourfit_date.hour = 22;
  my200.fourfit_date.minute = 22;
  my200.fourfit_date.second = 22;
  my200.frt.year = 2022;
  my200.frt.day = 22;
  my200.frt.hour = 22;
  my200.frt.minute = 22;
  my200.frt.second = 22;
  //fillType200(&my200);

  json obj = convertToJSON(my200);

  std::cout << obj.dump(2) << std::endl;

  return 0;
}
