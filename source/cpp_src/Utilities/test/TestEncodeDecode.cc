#include <vector>
#include <string>
#include <iostream>

#include "MHO_EncodeDecodeValue.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    std::string base10("0123456789");
    std::string base16("0123456789ABCDEF");
    std::string base64("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$%");
    std::string base32("abcdefghijklmnopqrstuvwxyzABCDEF");
    std::string base26("abcdefghijklmnopqrstuvwxyz");
    std::string base52("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

    std::vector< std::string > bases;
    bases.push_back(base10);
    bases.push_back(base16);
    bases.push_back(base26);
    bases.push_back(base32);
    bases.push_back(base52);
    bases.push_back(base64);

    std::vector<uint64_t> values = 
    {
        0,1,10,
        15,16,17,
        25,26,27,
        31,32,33,
        63,64,65,
        51,52,53,
        99,100,101,
        255,256,257,
        675,676,677,
        2703,2704,2705,
        4095,4096,4097
    };

    for(std::size_t b=0; b<bases.size(); b++)
    {
        for(std::size_t i=0; i<values.size(); i++)
        {
            std::string enc_val = encode_value(values[i], bases[b]);
            uint64_t dec_val = decode_value(enc_val, bases[b]);
            uint64_t delta = dec_val - values[i];
            if(delta != 0){std::cout<<"encode/decode error."<<std::endl; return 1;}

            std::cout<<"base: "<<bases[b].size()<<" (value, code, decode) = ("
            <<values[i]<<","<<enc_val<<","<<dec_val<<")"<<std::endl;
        }
    }

    return 0;
}
