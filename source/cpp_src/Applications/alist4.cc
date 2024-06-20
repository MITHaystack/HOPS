#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <getopt.h>

//option parsing and help text library
#include "CLI11.hpp"

#define EXTRA_DEBUG

#include "MHO_Message.hh"
#include "MHO_ContainerDefinitions.hh"

//needed to export to mark4 fringe files
#include "MHO_MK4FringeExport.hh"
#include "MHO_MPIInterfaceWrapper.hh"


using namespace hops;


int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();


    return 0;
}
