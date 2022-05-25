#ifndef MHO_VexBlockDefinitions_HH__
#define MHO_VexBlockDefinitions_HH__

/*
*@file: MHO_VexBlockDefinitions.hh
*@class: MHO_VexBlockDefinitions
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include "MHO_Message.hh"

namespace hops 
{

class MHO_VexBlockDefinitions
{
    public:

        MHO_VexBlockDefinitions(){};
        virtual ~MHO_VexBlockDefinitions(){};

        static
        std::vector<std::string> 
        GetBlockNames( std::string version)
        {
            std::vector< std::string > v1p5blocks = 
            {
                "$GLOBAL", "$STATION", "$MODE", "$SCHED", "$ANTENNA", "$BBC",
                "$CLOCK", "$DAS", "$DATASTREAMS", "$EOP", "$EXPER", "$FREQ",
                "$HEAD_POS", "$IF", "$PASS_ORDER", "$PHASE_CAL_DETECT",
                "$PROCEDURES", "$ROLL", "$SCHEDULING_PARAMS", "$SEFD", "$SITE",
                "$SOURCE", "$TAPELOG_OBS", "$TRACKS"
            };

            std::vector< std::string > v2blocks = 
            {
                "$GLOBAL", "$STATION", "$MODE", "$SCHED", "$ANTENNA", "$BBC",
                "$BITSTREAMS", "$CLOCK", "$DAS", "$DATASTREAMS",
                "$EOP", "$EXPER", "$EXTENSIONS","$FREQ", "$IF",
                "$PHASE_CAL_DETECT", "$PROCEDURES", "$SCHEDULING_PARAMS",
                "$SEFD", "$SITE", "$SOURCE", "$TAPELOG_OBS", "$TRACKS"
            };

            if(version == "1.5"){return v1p5blocks;}
            if(version == "2.0"){return v2blocks;}

            msg_error("vex", "version string: "<<version<<" not understood, cannot process vex file." << eom);

            return std::vector<std::string>();
        }

};


}//end namespace 

#endif /* end of include guard: MHO_VexBlockDefinitions */