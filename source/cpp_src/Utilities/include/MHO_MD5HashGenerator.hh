#ifndef MHO_MD5HashGenerator_HH__
#define MHO_MD5HashGenerator_HH__



#include <stdio.h>
#include <stdint.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <complex>

#include "picohash.h"
#include "MHO_UUID.hh"


namespace hops
{

/*!
*@file MHO_MD5HashGenerator.hh
*@class MHO_MD5HashGenerator
*@author J. Barrett - barrettj@mit.edu
*@date Wed Apr 21 13:40:18 2021 -0400
*@brief
*/

//forward declare our md5 hashing streamer (for plain old data types ((POD))
class MHO_MD5HashGenerator;

//template class for a single-type streamer, generic for most POD types
template<typename XValueType>
class MHO_MD5HashGeneratorSingleType
{
    public:

        MHO_MD5HashGeneratorSingleType(){};
        virtual ~MHO_MD5HashGeneratorSingleType(){};

        friend inline MHO_MD5HashGenerator& operator<<(MHO_MD5HashGeneratorSingleType<XValueType>& s, const XValueType& obj)
        {
            //run the hash update
            _picohash_md5_update( s.GetMD5CTXPtr(), reinterpret_cast<const void*>(&obj), sizeof(XValueType));
            return s.Self();
        }

    protected:

        virtual _picohash_md5_ctx_t* GetMD5CTXPtr() = 0;
        virtual MHO_MD5HashGenerator& Self() = 0;
};


//specialization for string type
template<> class MHO_MD5HashGeneratorSingleType< std::string >
{
    public:
        MHO_MD5HashGeneratorSingleType(){};
        virtual ~MHO_MD5HashGeneratorSingleType(){};

        friend inline MHO_MD5HashGenerator& operator<<(MHO_MD5HashGeneratorSingleType<std::string>& s, std::string& obj)
        {
            char ch;
            for(unsigned int i = 0; i < obj.size(); i++)
            {
                ch = obj.at(i);
                _picohash_md5_update( s.GetMD5CTXPtr(), reinterpret_cast<const void*>(&ch), sizeof(char) );
            }
            return s.Self();
        }

    protected:

        virtual _picohash_md5_ctx_t* GetMD5CTXPtr() = 0;
        virtual MHO_MD5HashGenerator& Self() = 0;
};

//now declare a multi-type streamer with a variadic template parameter for the types to be streamed
template <typename... XValueTypeS>
class MHO_MD5HashGeneratorMultiType;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
template <typename XValueType>
class MHO_MD5HashGeneratorMultiType< XValueType >: public MHO_MD5HashGeneratorSingleType< XValueType > {};

template<typename XValueType, typename... XValueTypeS >
class MHO_MD5HashGeneratorMultiType< XValueType, XValueTypeS...>:
public MHO_MD5HashGeneratorMultiType< XValueType >,
public MHO_MD5HashGeneratorMultiType< XValueTypeS... >{};

//construct the multi-type streamer for most basic POD types
typedef MHO_MD5HashGeneratorMultiType<
    bool,
    char,
    unsigned char,
    short,
    unsigned short,
    int,
    unsigned int,
    long,
    unsigned long,
    long long,
    unsigned long long,
    float,
    double,
    long double,
    std::string
    >
MHO_MD5HashGeneratorBasicTypes;

//now declare the concrete class which does the work for file streams
class MHO_MD5HashGenerator: public MHO_MD5HashGeneratorBasicTypes
{
    public:

        MHO_MD5HashGenerator()
        {
            Initialize();
        };

        virtual ~MHO_MD5HashGenerator(){};

        void Initialize()
        {
            _picohash_md5_init(&fHashStruct);
            for(unsigned int i=0; i<PICOHASH_MD5_DIGEST_LENGTH; i++){fDigest[i] = 0;}
        }

        void Finalize()
        {
            _picohash_md5_final(&fHashStruct, (void*) fDigest);
        }

        std::string GetDigest()
        {
            std::stringstream ss;
            for(unsigned int i=0; i<PICOHASH_MD5_DIGEST_LENGTH; i++)
            {
                uint32_t tmp = fDigest[i];
                std::stringstream hss;
                hss << std::setw(2) << std::setfill('0') << std::hex << (int)( tmp );
                std::string hexstr = hss.str();
                ss << hexstr;
            }
            return ss.str();
        }

        MHO_UUID GetDigestAsUUID()
        {
            MHO_UUID uuid;
            for(unsigned int i=0; i<PICOHASH_MD5_DIGEST_LENGTH; i++)
            {
                uuid[i] = fDigest[i];
            }
            return uuid;
        }

    protected:

        virtual _picohash_md5_ctx_t* GetMD5CTXPtr() override {return &fHashStruct;}
        MHO_MD5HashGenerator& Self() override {return *this;}

        _picohash_md5_ctx_t fHashStruct;
        uint8_t fDigest[PICOHASH_MD5_DIGEST_LENGTH];
};



}//end of hops namespace

#endif /*! end of include guard: MHO_MD5HashGenerator */
