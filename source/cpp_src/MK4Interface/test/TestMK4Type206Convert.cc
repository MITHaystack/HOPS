#include <iostream>
#include <string>
#include <stdlib.h>
#include "MHO_MK4Type206Converter.hh"

const int NUMBEROFSIDEBANDSANDSBWEIGHTS = 64;

using namespace hops;

int main(int argc, char** argv) {
    struct type_206 my206;

    // create and fill in a type_206 struct with some dummy data 
    strcpy(my206.record_id, "202"); 
    strcpy(my206.version_no, "2"); 
    strcpy(my206.unused1, "unused..."); 
    my206.start.year = 3033;
    my206.start.day = 33;
    my206.start.hour = 33;
    my206.start.minute = 33;
    my206.start.second = 33;
    my206.first_ap = 22;
    my206.last_ap = 22;
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.accepted[i].lsb = 4;
      my206.accepted[i].usb = 4;
    }
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.weights[i].lsb = 4;
      my206.weights[i].usb = 4;
    }
    my206.intg_time = 22.22;
    my206.accept_ratio = 22.22;
    my206.discard = 22.22;
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.reason1[i].lsb = 4;
      my206.reason1[i].usb = 4;
    }
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.reason2[i].lsb = 4;
      my206.reason2[i].usb = 4;
    }
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.reason3[i].lsb = 4;
      my206.reason3[i].usb = 4;
    }
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.reason4[i].lsb = 4;
      my206.reason4[i].usb = 4;
    }
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.reason5[i].lsb = 4;
      my206.reason5[i].usb = 4;
    }
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.reason6[i].lsb = 4;
      my206.reason6[i].usb = 4;
    }
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.reason7[i].lsb = 4;
      my206.reason7[i].usb = 4;
    }
    for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++){
      my206.reason8[i].lsb = 4;
      my206.reason8[i].usb = 4;
    }
    my206.ratesize = 2;
    my206.mbdsize = 2;
    my206.sbdsize = 2;
    strcpy(my206.unused2, "unuse"); 

    json obj = convertToJSON(my206);
    std::cout << obj.dump(2) << std::endl;

    return 0;
}
