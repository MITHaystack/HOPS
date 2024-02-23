#ifndef MHO_EncodeValue_HH__
#define MHO_EncodeValue_HH__


/*!
*@file MHO_EncodeValue.hh
*@class
*@date
*@brief
*@author J. Barrett - barrettj@mit.edu
*/

#include <string>
#include <stack>
#include <limits>

#include "MHO_Types.hh"

//functions to encode/decode an unsigned base-10 integer value in and out of
//another base, represented by the single-character symbols present in
//the specified character_set.

//for example, to encode a value, x, in standard base-16, one would call:
//std::string eval = encode_value(x, std::string("0123456789ABCDEF") );
//similarly to decode a base-16 encoded string, y, to a unsigned base-10 integer,
//one would call:
//uint64_t value = decode_value(y, std::string("0123456789ABCDEF") );

//these functions are not particularly performant

namespace hops
{


std::string
encode_value(const uint64_t& value, const std::string& character_set)
{
    std::string encoded_value = "";
    std::stack<char> cstack;
    uint64_t base = character_set.size();
    if(value >= 0)
    {
        uint64_t q,r;
        q = value;
        do
        {
            r = q%base;
            q = q/base;
            cstack.push(character_set[r]);
        }
        while( q > 0);

        while(cstack.size() != 0)
        {
            encoded_value += cstack.top();
            cstack.pop();
        }
    }
    return encoded_value;
}

uint64_t
decode_value(const std::string& code, const std::string& character_set)
{
    uint64_t decoded_value = 0;
    std::stack<char> cstack;
    uint64_t base = character_set.size();
    uint64_t bpower = 1;
    for(auto it=code.rbegin(); it != code.rend(); it++)
    {
        uint64_t place_value = character_set.find(*it);
        if(place_value != std::string::npos )
        {
            decoded_value += place_value*bpower;
            bpower *= base;
        }
        else
        {
            //out of range error
            return std::numeric_limits<uint64_t>::max();
        }
    }
    return decoded_value;
}




}//end namespace

#endif /*! end of include guard: MHO_EncodeValue_HH__ */
