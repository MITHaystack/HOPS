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
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace hops
{

#define MHO_UUID_LENGTH 16

class MHO_UUID
{

    public:

        MHO_UUID()
        {
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++){fBytes[i] = 0;}
        }
        virtual ~MHO_UUID(){};


        MHO_UUID(const MHO_UUID& copy)
        {
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++){fBytes[i] = copy[i];}
        }


        MHO_UUID& operator=(const MHO_UUID& rhs)
        {
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++){fBytes[i] = rhs[i];}
            return *this;
        }

        //access operator
        uint8_t& operator[](std::size_t i){return fBytes[i];}
        const uint8_t& operator[](std::size_t i) const {return fBytes[i];}

        bool operator==(const MHO_UUID& rhs) const
        {
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++)
            {
                if(fBytes[i] != rhs[i]){return false;}
            }
            return true;
        }

        bool operator!=(const MHO_UUID& rhs) const
        {
            return !(*this == rhs);
        }

        bool operator<(const MHO_UUID& rhs) const
        {
            //use STL lexi compare
            return std::lexicographical_compare(&(fBytes[0]),
                                                &(fBytes[MHO_UUID_LENGTH-1]),
                                                &(rhs[0]),
                                                &(rhs[MHO_UUID_LENGTH-1]));
        }

        std::string as_string() const
        {
            std::stringstream ss;
            for(unsigned int i=0; i<MHO_UUID_LENGTH; i++)
            {
                uint32_t tmp = fBytes[i];
                std::stringstream hss;
                hss << std::setw(2) << std::setfill('0') << std::hex << (int)( tmp );
                std::string hexstr = hss.str();
                ss << hexstr;
            }
            return ss.str();
        }

        uint64_t size() const
        {
            return MHO_UUID_LENGTH;
        }

    protected:

        uint8_t fBytes[MHO_UUID_LENGTH];

};

template<typename XStream> XStream& operator>>(XStream& s, MHO_UUID& uuid)
{
    for(std::size_t i=0; i<MHO_UUID_LENGTH; i++)
    {
        s >> uuid[i];
    }
    return s;
};

template<typename XStream> XStream& operator<<(XStream& s, const MHO_UUID& uuid)
{
    for(std::size_t i=0; i<MHO_UUID_LENGTH; i++)
    {
        s << uuid[i];
    }
    return s;
};



}

#endif /* end of include guard: MHO_UUID */
