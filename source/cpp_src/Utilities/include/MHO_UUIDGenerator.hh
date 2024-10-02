#ifndef MHO_UUIDGenerator_HH__
#define MHO_UUIDGenerator_HH__

#include "MHO_UUID.hh"
#include <random>
#include <sstream>
#include <string>

namespace hops
{

/*!
 *@file MHO_UUIDGenerator.hh
 *@class MHO_UUIDGenerator
 *@date Wed Apr 21 13:40:18 2021 -0400
 *@author J. Barrett - barrettj@mit.edu
 * A class to provide pseudo-randomly generated UUID strings,
 * based on RFC-4122, but with a non-standard 4 byte sync word as a prefix
 * https://tools.ietf.org/html/rfc4122
 */

class MHO_UUIDGenerator
{
    public:
        MHO_UUIDGenerator();
        virtual ~MHO_UUIDGenerator();

        /*!* Generate a 128 bit psuedo-random UUID by following the RFC 4122 standard
         * @param None
         * @returns uuid MHO_UUID
         */
        MHO_UUID GenerateUUID();

        /*!* Convert the UUID to a string
         * @param uuid MHO_UUID& which is a UUID passed by reference
         * @returns ss string which is the string version of the UUID
         */
        std::string ConvertUUIDToString(MHO_UUID& uuid);

        /*!* Create a MHO_UUID UUID and convert it to a string
         * @param None
         * @returns uuid string
         */
        std::string GenerateUUIDAsString();

    private:
        uint8_t fSyncWord[4]; //32 bits of sync word

        std::random_device fRandomDev;
        std::mt19937* fRandomGen;
        std::uniform_int_distribution< uint32_t >* fUniformDist;
};

} // namespace hops

#endif /*! end of include guard: MHO_UUIDGenerator */
