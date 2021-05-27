#ifndef MHO_StationStructWrapper_HH__
#define MHO_StationStructWrapper_HH__


/*
*File: MHO_StationStructWrapper.hh
*Class: MHO_StationStructWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_JSONOutputObject.hh"

extern "C"
{
#include "ovex.h"
}


namespace hops
{

class MHO_StationStructWrapper: public MHO_JSONOutputObject
{
    public:

        MHO_StationStructWrapper(station_struct aStation)
        {
            fStation = aStation;
        };

        MHO_StationStructWrapper(){};

        virtual ~MHO_StationStructWrapper(){};

        MHO_StationStructWrapper(const MHO_StationStructWrapper& copyObject)
        {
            fStation = copyObject.fStation;
        };

        MHO_StationStructWrapper& operator=(const MHO_StationStructWrapper& rhs)
        {
            if(&rhs != this)
            {
                fStation = rhs.fStation;
            }
            return *this;
        };

        virtual void DumpToJSON(json& json_obj);

        virtual const char* GetName() const {return "station_struct";};
        virtual const char* ClassName() const { return "station_struct"; };

    public:

        station_struct fStation;

};


}


#endif /* end of include guard: MHO_StationStructWrapper */
