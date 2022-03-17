#include <iostream>
#include <string>

#include "MHO_MK4Type200Converter.hh"


using namespace hops;


int main(int argc, char** argv)
{

    //create and fill in a type_200 struct with some dummy data 
    struct type_200 my200;

    //fill the record id array
    my200.record_id[0] = '2'; 
    my200.record_id[1] = '0';
    my200.record_id[2] = '0';

    //fill the version array
    my200.version_no[0] = '0'; 
    my200.version_no[1] = '0';

    for(int i=0; i<10; i++) {
        my200.software_rev[i] = 1; //I have no idea what this field is actually used for
    }
    my200.expt_no = 1234;
    my200.exper_name[0] = 'a'; 
    my200.scan_name[0] = 'b';  
    my200.correlator[0] = 'c';
    my200.start_offset = 0;
    my200.stop_offset = 1;

    //all of the date objects (scantime, corr_date, fourfit_date, frt) are also structs...skip for now
    //so we should figure out what we want to do with these later


    MHO_MKType200Converter myConverter;
    myConverter.SetType200(&my200);
    myConverter.ConvertToJSON();
    
    json obj = myConverter.GetJSON();

    //now print to screen 

    std::cout<< obj.dump(2) << std::endl;
    

    return 0;
}
