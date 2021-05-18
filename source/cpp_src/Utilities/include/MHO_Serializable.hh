#ifndef MHO_Serializable_HH__
#define MHO_Serializable_HH__

/*
*File: MHO_Serializable.hh
*Class: MHO_Serializable
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:21:32.924Z
*Description:
*/

#include <cstdint>

#include "MHO_ClassIdentity.hh"
#include "MHO_FileStreamer.hh"

namespace hops
{

class MHO_Serializable
{
    public:
        MHO_Serializable(){};
        virtual ~MHO_Serializable(){};

        virtual MHO_ClassVersion GetVersion() const {return 0;};
        virtual uint64_t GetSerializedSize() const = 0;
};

}  // namespace hops

#endif /* end of include guard: MHO_Serializable */
