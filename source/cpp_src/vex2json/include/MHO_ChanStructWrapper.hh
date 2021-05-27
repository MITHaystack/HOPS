#ifndef MHO_ChanStructWrapper_HH__
#define MHO_ChanStructWrapper_HH__

#include "MHO_JSONOutputObject.hh"

extern "C"
{
#include "ovex.h"
}


/*
*File: MHO_ChanStructWrapper.hh
*Class: MHO_MHO_ChanStructWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

// struct chan_struct
//     {
//     char                chan_name[32];          /* External channel name */
//     char                polarization;           /* R or L */
//     double              sky_frequency;          /* Hz */
//     char                net_sideband;           /* U or L */
//     double              bandwidth;              /* Hz */
//     char                band_id[32];            /* Linkword (internal use) */
//     char                chan_id[32];            /* Linkword (internal use) */
//     char                bbc_id[32];             /* Linkword (internal use) */
//     char                pcal_id[32];            /* Linkword (internal use) */
//     char                if_id[32];              /* Linkword (internal use) */
//     short               bbc_no;                 /* Physical BBC# */
//     char                if_name[8];             /* Physical IF name */
//     double              if_total_lo;            /* Hz */
//     char                if_sideband;            /* U or L */
//     float               pcal_spacing;           /* Hz */
//     float               pcal_base_freq;         /* Hz */
//     short               pcal_detect[16];        /* Integer tone #s */
//     short               sign_tracks[4];         /* Track #s */
//     short               sign_headstack;         /* 1-4 */
//     short               mag_tracks[4];          /* Track #s */
//     short               mag_headstack;          /* 1-4 */
//     };




namespace hops
{

class MHO_ChanStructWrapper: public MHO_JSONOutputObject
{
    public:

        MHO_ChanStructWrapper(chan_struct aChan)
        {
            fChan = aChan;
        };

        MHO_ChanStructWrapper(){};

        virtual ~MHO_ChanStructWrapper(){};

        MHO_ChanStructWrapper(const MHO_ChanStructWrapper& copyObject)
        {
            fChan = copyObject.fChan;
        };

        MHO_ChanStructWrapper& operator=(const MHO_ChanStructWrapper& rhs)
        {
            if(&rhs != this)
            {
                fChan = rhs.fChan;
            }
            return *this;
        };

        virtual void DumpToJSON(json& json_obj);

        virtual const char* GetName() const {return "chan_struct";};
        virtual const char* ClassName() const { return "chan_struct"; };

    public:

        chan_struct fChan;

};


}

#endif /* end of include guard: MHO_ChanStructWrapper */
