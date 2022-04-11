#include <iostream>
#include <string>

#include "MHO_MK4Type200Converter.hh"


using namespace hops;


int main(int argc, char** argv)
{

    //create and fill in a type_200 struct with some dummy data 
    struct type_202 my202;

    //fill the record id array
    my202.record_id[0] = '2'; 
    my202.record_id[1] = '0';
    my202.record_id[2] = '2';

    //fill the version array
    my202.version_no[0] = '0'; 
    my202.version_no[1] = '0';

    strcpy(my202.baseline, "aa"); 
    strcpy(my202.ref_intl_id, "bb"); 
    strcpy(my202.rem_intl_id, "cc"); 
    strcpy(my202.ref_name, "abcdefgh"); 
    strcpy(my202.rem_name, "jklmnopq"); 
    strcpy(my202.ref_tape, "jklmnopq"); 
    strcpy(my202.rem_tape, "jklmnopq"); 
    my202.nlags[0] = 'z';
    my202.ref_ypos = 2;
    my202.rem_ypos = 2;
    my202.ref_xpos = 1;
    my202.rem_xpos = 2;
    my202.ref_zpos = 2;
    my202.rem_zpos = 2;
    my202.ref_ypos = 2;
    my202.ref_clock = 2;
    my202.rem_clock = 2;
    my202.ref_clockrate = 2;
    my202.rem_clockrate = 2;
    my202.rem_idelay = 2;
    my202.ref_idelay = 2;
    my202.ref_zdelay = 2;
    my202.rem_zdelay = 2;
    my202.ref_elev = 2;
    my202.rem_elev = 2;
    my202.u = 2;
    my202.v = 2;
    my202.uf = 2;
    my202.vf = 2;

    json obj = convertToJSON(my200);

    std::cout << obj.dump(2) << std::endl;


    return 0;
}
