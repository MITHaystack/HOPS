#include <iostream>
#include <string>

#include "MHO_MK4Type203Converter.hh"


using namespace hops;


int main(int argc, char** argv)
{

    //create and fill in a type_203 struct with some dummy data 
    struct type_203 my203;

    strcpy(my203.record_id, "202"); 
    strcpy(my203.version_no, "000"); 
    strcpy(my203.ref_name, "abcdefgh"); 
    strcpy(my203.rem_name, "jklmnopq"); 
    strcpy(my203.ref_tape, "jklmnopq"); 
    strcpy(my203.rem_tape, "jklmnopq"); 

    json obj = convertToJSON(my203);

    std::cout << obj.dump(2) << std::endl;


    return 0;
}
