#include "MHOMK4StationInterface.hh"

#include <vector>
#include <cstdlib>
#include <cstring>
#include <complex>
#include <set>
#include <algorithm>

//mk4 IO library
extern "C"
{
    #include "mk4_records.h"
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}


namespace hops
{


MHOMK4StationInterface::MHOMK4StationInterface()
{

}

MHOMK4StationInterface::~MHOMK4StationInterface()
{

}



}//end of namespace
