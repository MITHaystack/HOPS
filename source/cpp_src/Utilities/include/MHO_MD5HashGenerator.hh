#ifndef MHO_MD5HashGenerator_HH__
#define MHO_MD5HashGenerator_HH__

#include <complex>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string>

#include "MHO_UUID.hh"
#include "picohash.h"

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
/**
 * @brief Class MHO_MD5HashGenerator
 */
class MHO_MD5HashGenerator;

//template class for a single-type streamer, generic for most POD types
/**
 * @brief Class MHO_MD5HashGeneratorSingleType
 */
template< typename XValueType > class MHO_MD5HashGeneratorSingleType
{
    public:
        MHO_MD5HashGeneratorSingleType(){};
        virtual ~MHO_MD5HashGeneratorSingleType(){};

        friend inline MHO_MD5HashGenerator& operator<<(MHO_MD5HashGeneratorSingleType< XValueType >& s, const XValueType& obj)
        {
            //run the hash update
            _picohash_md5_update(s.GetMD5CTXPtr(), reinterpret_cast< const void* >(&obj), sizeof(XValueType));
            return s.Self();
        }

    protected:
        /**
         * @brief Getter for md5ctxptr
         * 
         * @return _picohash_md5_ctx_t* Pointer to the MD5 context structure.
         * @note This is a virtual function.
         */
        virtual _picohash_md5_ctx_t* GetMD5CTXPtr() = 0;
        /**
         * @brief Returns a reference to the current instance of MHO_MD5HashGenerator.
         * 
         * @return MHO_MD5HashGenerator&: Reference to the current instance
         * @note This is a virtual function.
         */
        virtual MHO_MD5HashGenerator& Self() = 0;
};

//specialization for string type
/**
 * @brief Class MHO_MD5HashGeneratorSingleType<std::string>
 */
template<> class MHO_MD5HashGeneratorSingleType< std::string >
{
    public:
        MHO_MD5HashGeneratorSingleType(){};
        virtual ~MHO_MD5HashGeneratorSingleType(){};

        friend inline MHO_MD5HashGenerator& operator<<(MHO_MD5HashGeneratorSingleType< std::string >& s, std::string& obj)
        {
            char ch;
            for(unsigned int i = 0; i < obj.size(); i++)
            {
                ch = obj.at(i);
                _picohash_md5_update(s.GetMD5CTXPtr(), reinterpret_cast< const void* >(&ch), sizeof(char));
            }
            return s.Self();
        }

    protected:
        /**
         * @brief Getter for md5ctxptr
         * 
         * @return _picohash_md5_ctx_t* Pointer to the MD5 context structure.
         * @note This is a virtual function.
         */
        virtual _picohash_md5_ctx_t* GetMD5CTXPtr() = 0;
        /**
         * @brief Returns a reference to the current instance of MHO_MD5HashGenerator.
         * 
         * @return MHO_MD5HashGenerator&: Reference to the current instance
         * @note This is a virtual function.
         */
        virtual MHO_MD5HashGenerator& Self() = 0;
};

//now declare a multi-type streamer with a variadic template parameter for the types to be streamed
/**
 * @brief Class MHO_MD5HashGeneratorMultiType
 */
template< typename... XValueTypeS > class MHO_MD5HashGeneratorMultiType;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
/**
 * @brief Class MHO_MD5HashGeneratorMultiType<XValueType>
 * 
 * @tparam XValueTypeS Template parameter XValueTypeS
 */
template< typename XValueType >
class MHO_MD5HashGeneratorMultiType< XValueType >: public MHO_MD5HashGeneratorSingleType< XValueType >
{};

/**
 * @brief Class MHO_MD5HashGeneratorMultiType<XValueType, XValueTypeS...>
 */
template< typename XValueType, typename... XValueTypeS >
class MHO_MD5HashGeneratorMultiType< XValueType, XValueTypeS... >: public MHO_MD5HashGeneratorMultiType< XValueType >,
                                                                   public MHO_MD5HashGeneratorMultiType< XValueTypeS... >
{};

//construct the multi-type streamer for most basic POD types
typedef MHO_MD5HashGeneratorMultiType< bool, char, unsigned char, short, unsigned short, int, unsigned int, long, unsigned long,
                                       long long, unsigned long long, float, double, long double, std::string >
    MHO_MD5HashGeneratorBasicTypes;

//now declare the concrete class which does the work for file streams
/**
 * @brief Class MHO_MD5HashGenerator
 */
class MHO_MD5HashGenerator: public MHO_MD5HashGeneratorBasicTypes
{
    public:
        MHO_MD5HashGenerator() { Initialize(); };

        virtual ~MHO_MD5HashGenerator(){};

        /**
         * @brief Recursive function to initialize a game state.
         */
        void Initialize()
        {
            _picohash_md5_init(&fHashStruct);
            for(unsigned int i = 0; i < PICOHASH_MD5_DIGEST_LENGTH; i++)
            {
                fDigest[i] = 0;
            }
        }

        /**
         * @brief Finalizes the MD5 hash calculation and stores the result in fDigest.
         */
        void Finalize() { _picohash_md5_final(&fHashStruct, (void*)fDigest); }

        /**
         * @brief Getter for digest
         * 
         * @return MD5 digest as a hexadecimal string
         */
        std::string GetDigest()
        {
            std::stringstream ss;
            for(unsigned int i = 0; i < PICOHASH_MD5_DIGEST_LENGTH; i++)
            {
                uint32_t tmp = fDigest[i];
                std::stringstream hss;
                hss << std::setw(2) << std::setfill('0') << std::hex << (int)(tmp);
                std::string hexstr = hss.str();
                ss << hexstr;
            }
            return ss.str();
        }

        /**
         * @brief Getter for digest as uuid
         * 
         * @return MHO_UUID representing the MD5 digest
         */
        MHO_UUID GetDigestAsUUID()
        {
            MHO_UUID uuid;
            for(unsigned int i = 0; i < PICOHASH_MD5_DIGEST_LENGTH; i++)
            {
                uuid[i] = fDigest[i];
            }
            return uuid;
        }

    protected:
        /**
         * @brief Getter for md5ctxptr
         * 
         * @return _picohash_md5_ctx_t* Pointer to the MD5 context structure.
         * @note This is a virtual function.
         */
        virtual _picohash_md5_ctx_t* GetMD5CTXPtr() override { return &fHashStruct; }

        /**
         * @brief Returns a reference to the current instance of MHO_MD5HashGenerator.
         * 
         * @return MHO_MD5HashGenerator&: Reference to the current instance
         */
        MHO_MD5HashGenerator& Self() override { return *this; }

        _picohash_md5_ctx_t fHashStruct;
        uint8_t fDigest[PICOHASH_MD5_DIGEST_LENGTH];
};

} // namespace hops

#endif /*! end of include guard: MHO_MD5HashGenerator */
