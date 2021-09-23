#ifndef MHO_JSONOutputObject_HH__
#define MHO_JSONOutputObject_HH__

#include "MHO_JSONHeaderWrapper.hh"

extern "C"
{
#include "ovex.h"
}



/*
*File: MHO_JSONOutputObject.hh
*Class: MHO_JSONOutputObject
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{


class MHO_JSONOutputObject
{
    public:

        MHO_JSONOutputObject(){};
        virtual ~MHO_JSONOutputObject(){};

        virtual void DumpToJSON(json& /*json_obj*/){};

    protected:

        bool is_float_unset(float x)
        {
            float undef = F_UNDEFINED;
            if( fabs( (x - undef)/undef ) < 1e-6 ){return true;}
            return false;
        }

        bool is_short_unset(short x)
        {
            if(x == I_UNDEFINED){return true;}
            return false;
        }

        bool is_int_unset(int x)
        {
            if(x == I_UNDEFINED){return true;}
            return false;
        }

};

} //end of namespace

#endif /* end of include guard: MHO_JSONOutputObject */
