#include <iostream>
#include <stdlib.h>
#include <string>

#include "MHO_MK4Type204Converter.hh"

using namespace hops;

int main(int argc, char **argv) {
  srand(time(NULL));

  struct type_204 my204;

  // create and fill in a type_204 struct with some dummy data
  strncpy(my204.record_id, "202", sizeof(my204.record_id));
  strncpy(my204.version_no, "000", sizeof(my204.version_no));
  my204.ff_version[0] = 2;
  my204.ff_version[1] = 2;
  strncpy(my204.platform, "Actually, it's GNU/Linux", sizeof(my204.platform));
  strncpy(my204.control_file, "foo.txt", sizeof(my204.control_file));
  my204.ffcf_date.year = 2022;
  my204.ffcf_date.day = 22;
  my204.ffcf_date.hour = 22;
  my204.ffcf_date.minute = 22;
  my204.ffcf_date.second = 22;
  strncpy(my204.override, "bar", sizeof(my204.override));

  json obj = convertToJSON(my204);
  std::cout << obj.dump(2) << std::endl;

  return 0;
}
