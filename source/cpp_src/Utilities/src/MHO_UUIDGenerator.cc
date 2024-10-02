#include "MHO_UUIDGenerator.hh"

#include <iomanip>

namespace hops
{

MHO_UUIDGenerator::MHO_UUIDGenerator()
{
    fRandomGen = new std::mt19937(fRandomDev());
    fUniformDist = new std::uniform_int_distribution< uint32_t >(0, 255);
}

MHO_UUIDGenerator::~MHO_UUIDGenerator()
{
    delete fUniformDist;
    delete fRandomGen;
};

MHO_UUID MHO_UUIDGenerator::GenerateUUID()
{
    MHO_UUID uuid;
    //generate the rest of the bits according to version-4
    //(128 bit) pseudo-random UUID based on RFC 4122
    for(unsigned int i = 0; i < MHO_UUID_LENGTH; i++)
    {
        //get a random 32 bit uint between 0 and 255
        uint32_t tmp = (*fUniformDist)(*fRandomGen);
        //set 4 MSB bits to 0100 for 7th byte
        if(i == 6)
        {
            tmp &= 0x0F;
            tmp |= 0x40;
        }
        //set 2 MSB bits to 01 for 9th byte
        if(i == 8)
        {
            tmp &= 0x3F;
            tmp |= 0x80;
        }
        uuid[i] = (uint8_t)tmp;
    }

    return uuid;
}

std::string MHO_UUIDGenerator::ConvertUUIDToString(MHO_UUID& uuid)
{
    std::stringstream ss;
    for(unsigned int i = 0; i < 16; i++)
    {
        uint32_t tmp = uuid[i];
        std::stringstream hss;
        hss << std::setw(2) << std::setfill('0') << std::hex << (int)(tmp);
        std::string hexstr = hss.str();
        ss << hexstr;
    }
    return ss.str();
}

std::string MHO_UUIDGenerator::GenerateUUIDAsString()
{
    MHO_UUID uuid = GenerateUUID();
    return ConvertUUIDToString(uuid);
}

} // namespace hops
