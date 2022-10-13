#include <iostream>
#include <string>

//#include "convertToJSON.hh"

using namespace hops;

int main(int argc, char **argv) {

  // create and fill in a type_200 struct with some dummy data
  struct type_200s my200s;

  // open file
  
  // call type 200s functions on type 200 structs from that file

  // store in my200s

  // dump to file
  std::ofstream outputFile(type200-dump.js);
  outputFile << std::setw(4) << my200s << std::endl;
  outputFile.close()

  return 0;
}
