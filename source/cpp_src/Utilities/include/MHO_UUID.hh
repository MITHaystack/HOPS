#ifndef MHO_UUID_HH__
#define MHO_UUID_HH__

/*
*File: MHO_UUID.hh
*Class: MHO_UUID
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cstdint>

namespace hops
{

#define MHO_UUID_LENGTH 16

struct MHO_UUID
{
    uint8_t fBytes[MHO_UUID_LENGTH];

    //access operator
    uint8_t& operator[](std::size_t i){return fBytes[i];}
    const uint8_t& operator[](std::size_t i) const {return fBytes[i];}

    bool operator==(const MHO_UUID& rhs)
    {
        for(std::size_t i=0; i<MHO_UUID_LENGTH; i++)
        {
            if(fBytes[i] != rhs.fBytes[i]){return false;}
        }
        return true;
    }

    bool operator!=(const MHO_UUID& rhs)
    {
        return !(*this == rhs);
    }


};

template<typename XStream> XStream& operator>>(XStream& s, MHO_UUID& uuid)
{
    for(std::size_t i=0; i<MHO_UUID_LENGTH; i++)
    {
        s >> uuid.fBytes[i];
    }
    return s;
};

template<typename XStream> XStream& operator<<(XStream& s, const MHO_UUID& uuid)
{
    for(std::size_t i=0; i<MHO_UUID_LENGTH; i++)
    {
        s << uuid.fBytes[i];
    }
    return s;
};


}

#endif /* end of include guard: MHO_UUID */
