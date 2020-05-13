#include <iostream>
#include <string>

#include "HTimeStampConverter.hh"
#include "HMultiTypeMap.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    HMultiTypeMap< std::string, int, double, float > myMap;

    std::string key1("key1");
    int val1 = 1;
    myMap.insert( key1, val1);

    return 0;
}
