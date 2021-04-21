#ifndef MHO_UUIDGenerator_HH__
#define MHO_UUIDGenerator_HH__

/*
*File: MHO_UUIDGenerator.hh
*Class: MHO_UUIDGenerator
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Class to provide pseudo-randomly generated UUID strings,
* based on RFC-4122, but with a non-standard 4 byte sync word as a prefix
*https://tools.ietf.org/html/rfc4122
*/



#include <sstream>
#include <random>
#include <string>
#include "MHO_UUID.hh"

namespace hops
{

class MHO_UUIDGenerator
{
    public:
        MHO_UUIDGenerator();
        virtual ~MHO_UUIDGenerator();

        MHO_UUID GenerateUUID();
        std::string ConvertUUIDToString(MHO_UUID& uuid);
        std::string GenerateUUIDAsString();

    private:

        uint8_t fSyncWord[4]; //32 bits of sync word

        std::random_device fRandomDev;
        std::mt19937* fRandomGen;
        std::uniform_int_distribution<uint32_t>* fUniformDist;

};

}//end of hops namespace

#endif /* end of include guard: MHO_UUIDGenerator */
