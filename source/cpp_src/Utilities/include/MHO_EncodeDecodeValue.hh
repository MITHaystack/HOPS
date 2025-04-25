#ifndef MHO_EncodeValue_HH__
#define MHO_EncodeValue_HH__

#include <limits>
#include <stack>
#include <string>

#include "MHO_Types.hh"

namespace hops
{

/*!
 *@file MHO_EncodeValue.hh
 *@class
 *@date Fri Jun 2 11:19:07 2023 -0400
 *@brief functions to encode/decode an unsigned base-10 integer value in and out of
 *another base, represented by the single-character symbols present in
 *the specified character_set.
 *for example, to encode a value, x, in standard base-16, one would call:
 *std::string eval = encode_value(x, std::string("0123456789ABCDEF") );
 *similarly to decode a base-16 encoded string, y, to a unsigned base-10 integer,
 *one would call:
 *uint64_t value = decode_value(y, std::string("0123456789ABCDEF") );
 *these functions are not particularly performant
 *@author J. Barrett - barrettj@mit.edu
 */

inline std::string encode_value(const uint64_t& value, const std::string& character_set)
{
    std::string encoded_value = "";
    std::stack< char > cstack;
    uint64_t base = character_set.size();
    if(value >= 0)
    {
        uint64_t q, r;
        q = value;
        do
        {
            r = q % base;
            q = q / base;
            cstack.push(character_set[r]);
        }
        while(q > 0);

        while(cstack.size() != 0)
        {
            encoded_value += cstack.top();
            cstack.pop();
        }
    }
    return encoded_value;
}

inline uint64_t decode_value(const std::string& code, const std::string& character_set)
{
    uint64_t decoded_value = 0;
    std::stack< char > cstack;
    uint64_t base = character_set.size();
    uint64_t bpower = 1;
    for(auto it = code.rbegin(); it != code.rend(); it++)
    {
        uint64_t place_value = character_set.find(*it);
        if(place_value != std::string::npos)
        {
            decoded_value += place_value * bpower;
            bpower *= base;
        }
        else
        {
            //out of range error
            return std::numeric_limits< uint64_t >::max();
        }
    }
    return decoded_value;
}

class MHO_ChannelIndexLabeler
{
    public:
        MHO_ChannelIndexLabeler()
        {
            //we inherited the set of 64 characters from fourfit
            //consider how we may want to change this in the future:
            fDefaultChannelChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$%";
            //this character set is used for >64 channels when constructing multi-character labels (don't use numbers + $%)
            fExtendedChannelChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        };

        virtual ~MHO_ChannelIndexLabeler(){};

        //provide the option to use different character sets
        void SetDefaultChannelChars(const std::string& ch_set) { fDefaultChannelChars = ch_set; }
        void SetExtendedChannelChars(const std::string& ex_set) { fExtendedChannelChars = ex_set; }

        //default encoding/decoding scheme
        std::string EncodeValueToLabel(const uint64_t& value) const
        {
            if(value < fDefaultChannelChars.size())
            {
                return encode_value(value, fDefaultChannelChars);
            }
            //else multi-char code
            uint64_t j = value - fDefaultChannelChars.size() + fExtendedChannelChars.size();
            return encode_value(j, fExtendedChannelChars);
        }

        uint64_t DecodeLabelToValue(const std::string& label) const
        {
            if(label.size() == 1)
            {
                return decode_value(label, fDefaultChannelChars);
            }
            //else multi-char code
            uint64_t extended_value = decode_value(label, fExtendedChannelChars);
            extended_value -= fExtendedChannelChars.size();
            extended_value += fDefaultChannelChars.size();
            return extended_value;
        }

    protected:

        //legal characters in channel labels
        std::string fDefaultChannelChars;
        std::string fExtendedChannelChars;
};




} // namespace hops

#endif /*! end of include guard: MHO_EncodeValue_HH__ */
