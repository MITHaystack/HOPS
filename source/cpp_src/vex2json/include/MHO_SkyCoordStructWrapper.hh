// typedef struct sky_coord
//     {
//     short       ra_hrs;                 /* Self-explanatory */
//     short       ra_mins;
//     float       ra_secs;
//     short       dec_degs;
//     short       dec_mins;
//     float       dec_secs;
//     } sky_coord_struct;

#ifndef MHO_SkyCoordStructWrapper_HH__
#define MHO_SkyCoordStructWrapper_HH__

#include "MHO_JSONOutputObject.hh"

extern "C"
{
    #include "mk4_typedefs.h"
}



/*
*File: MHO_SkyCoordStructWrapper.hh
*Class: MHO_SkyCoordStructWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Mon Jun 12 12:38:14 EDT 2017
*Description:
*/

namespace hops
{

class MHO_SkyCoordStructWrapper: public MHO_JSONOutputObject
{
    public:

        MHO_SkyCoordStructWrapper(sky_coord_struct aSkyCoord)
        {
            fSkyCoord = aSkyCoord;
        };

        MHO_SkyCoordStructWrapper(){};

        virtual ~MHO_SkyCoordStructWrapper(){};

        MHO_SkyCoordStructWrapper(const MHO_SkyCoordStructWrapper& copyObject)
        {
            fSkyCoord = copyObject.fSkyCoord;
        };

        MHO_SkyCoordStructWrapper& operator=(const MHO_SkyCoordStructWrapper& rhs)
        {
            if(&rhs != this)
            {
                fSkyCoord = rhs.fSkyCoord;
            }
            return *this;
        };

        virtual void DumpToJSON(json& json_obj);

        virtual const char* GetName() const {return "sky_coord_struct";};
        virtual const char* ClassName() const { return "sky_coord_struct"; };

    public:

        sky_coord_struct fSkyCoord;

};

}

#endif /* end of include guard: MHO_SkyCoordStructWrapper */
