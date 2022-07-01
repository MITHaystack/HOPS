#include <iostream>
#include <string>
#include <stdlib.h>

#include "MHO_MK4Type204Converter.hh"


using namespace hops;


int main(int argc, char** argv)
{
    srand (time(NULL));

    struct type_204 my204;

    // create and fill in a type_204 struct with some dummy data 
    strcpy(my204.record_id, "202"); 
    strcpy(my204.version_no, "000"); 
    my204.ff_version[0] = 2;
    my204.ff_version[1] = 2;
    strcpy(my204.platform, "Actually, it's GNU/Linux");
    strcpy(my204.control_file, "foo.txt");
    my204.ffcf_date.year = 2022;
    my204.ffcf_date.day = 22;
    my204.ffcf_date.hour = 22;
    my204.ffcf_date.minute = 22;
    my204.ffcf_date.second = 22;
    strcpy(my204.override, "bar");

    json obj = convertToJSON(my204);
    std::cout << obj.dump(2) << std::endl;

    return 0;
}
