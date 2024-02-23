#ifndef MHO_UUID_HH__
#define MHO_UUID_HH__

/*!
*@file MHO_UUID.hh
*@class MHO_UUID
*@author J. Barrett - barrettj@mit.edu 
*
*@date
*@brief
*/


#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "MHO_Types.hh"
#include "MHO_Message.hh"

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

        /*!*
        * Truncate the UUID byte array to the first (last) 8 bytes and convert into a 64 bit int
        * @return A uint64_t composed of the first or last 8 bytes of the UUID
        */
        uint64_t as_truncated_long(bool first_half=true) const
        {
            byte2ints b2i;
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++){b2i.byte_values[i] = fBytes[i];}
            if(first_half){return b2i.uint_values[0];}
            else{return b2i.uint_values[1];}
        }


        /*!*
        * Split the UUID byte array into two halves conver to uint64_t and return the sum
        * @return A uint64_t composed of the sum of the two halves of the uuid
        */
        uint64_t as_long() const
        {
            //stone knives and bear skins...
            byte2ints b2i;
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++){b2i.byte_values[i] = fBytes[i];}
            uint64_t result = b2i.uint_values[0];
            result += b2i.uint_values[1];
            return result;
        }


        /*!*
        * Convert the UUID byte array into a string
        * @return A std::string containing the hexadecimal digits of the UUID.
        */
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


        /*!*
        * Convert a formatted string into a UUID byte array and fill this object
        * @return update the uuid object from std::string containing the hexadecimal digits of the UUID.
        */
        bool from_string(const std::string& uuid_str)
        {
            if(uuid_str.size() == 2*MHO_UUID_LENGTH)
            {
                for(int i=0; i<MHO_UUID_LENGTH; i++)
                {
                    //one byte at a time
                    std::stringstream ss;
                    ss << uuid_str[2*i];
                    ss << uuid_str[2*i+1];
                    uint32_t val = std::strtoul(ss.str().c_str(), 0, 16);
                    fBytes[i] = val;
                }
                return true;
            }
            else
            {
                msg_error("utility", "could not convert string to uuid, length of " << uuid_str.size() << " != " <<2*MHO_UUID_LENGTH << " is incorrect" << eom );
                return false;
            }
        }

        uint64_t size() const
        {
            return MHO_UUID_LENGTH;
        }

        bool is_empty() const
        {
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++){ if(fBytes[i] != 0){return false;} }
            return true;
        }

        static uint64_t ByteSize(){return MHO_UUID_LENGTH*sizeof(uint8_t);};

    protected:

        typedef union
        {
            uint8_t byte_values[MHO_UUID_LENGTH];
            uint64_t uint_values[2];
        }
        byte2ints;

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

#endif /*! end of include guard: MHO_UUID */
