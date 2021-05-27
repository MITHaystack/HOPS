// typedef struct date
//     {
//     short	year;
//     short	day;
//     short	hour;
//     short	minute;
//     float	second;
//     } date_struct;

#ifndef MHO_DateStructWrapper_HH__
#define MHO_DateStructWrapper_HH__

#include "MHO_JSONOutputObject.hh"

extern "C"
{
    #include "mk4_typedefs.h"
}

/*
*File: MHO_DateStructWrapper.hh
*Class: MHO_DateStructWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Mon Jun 12 10:16:15 EDT 2017
*Description:
*/


namespace hops
{

class MHO_DateStructWrapper: public MHO_JSONOutputObject
{
    public:

        MHO_DateStructWrapper(date_struct aDate)
        {
            fDate = aDate;
        };

        MHO_DateStructWrapper(){};

        virtual ~MHO_DateStructWrapper(){};

        MHO_DateStructWrapper(const MHO_DateStructWrapper& copyObject)
        {
            fDate = copyObject.fDate;
        };

        MHO_DateStructWrapper& operator=(const MHO_DateStructWrapper& rhs)
        {
            if(&rhs != this)
            {
                fDate = rhs.fDate;
            }
            return *this;
        };

        virtual void DumpToJSON(json& json_obj);

        virtual const char* GetName() const {return "date_struct";};
        virtual const char* ClassName() const { return "date_struct"; };

    public:

        date_struct fDate;

};

}

#endif /* end of include guard: MHO_DateStructWrapper */
