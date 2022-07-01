#include <iostream>
#include <string>
#include <stdlib.h>
#include "MHO_MK4Type205Converter.hh"

const int NUMBEROFFFITCHAN = 16;

using namespace hops;


int main(int argc, char** argv)
{
    srand (time(NULL));

    struct type_205 my205;

    // create and fill in a type_205 struct with some dummy data 
    strcpy(my205.record_id, "202"); 
    strcpy(my205.version_no, "000"); 
    strcpy(my205.unused1, "i'm unused1"); 
    my205.utc_central.year = 2022;
    my205.utc_central.day = 11;
    my205.utc_central.hour = 16;
    my205.utc_central.minute = 0;
    my205.utc_central.second = 0;
    my205.offset = 2;
    strcpy(my205.ffmode, "a mode"); 
    my205.search[0] = 1;
    my205.search[1] = 1;
    my205.search[2] = 1;
    my205.search[3] = 1;
    my205.search[4] = 1;
    my205.search[5] = 1;
    my205.filter[0] = 2;
    my205.filter[1] = 2;
    my205.filter[2] = 2;
    my205.filter[3] = 2;
    my205.filter[4] = 2;
    my205.filter[5] = 2;
    my205.filter[6] = 2;
    my205.filter[7] = 2;
    my205.start.year = 2022;
    my205.start.day = 22;
    my205.start.hour = 22;
    my205.start.minute = 22;
    my205.start.second = 22;
    my205.stop.year = 3033;
    my205.stop.day = 33;
    my205.stop.hour = 33;
    my205.stop.minute = 33;
    my205.stop.second = 33;
    my205.ref_freq = 2;
    for (int i = 0; i < NUMBEROFFFITCHAN; i++){
        my205.ffit_chan[i].ffit_chan_id = 'a';
        my205.ffit_chan[i].unused = 'u';
        my205.ffit_chan[i].channels[0] = 0;
        my205.ffit_chan[i].channels[1] = 1;
        my205.ffit_chan[i].channels[2] = 2;
        my205.ffit_chan[i].channels[3] = 3;
    }

    json obj = convertToJSON(my205);
    std::cout << obj.dump(2) << std::endl;

    return 0;
}
