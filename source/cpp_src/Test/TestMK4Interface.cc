#include <iostream>
#include <string>

#include "HMK4Interface.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    std::string filename("/home/barrettj/geodesy/fringe-tests/3721/035-0002GH.X.15.10GN5W");

    HMK4Interface mk4inter;

    mk4inter.OpenFringeFile(filename);

    return 0;
}
