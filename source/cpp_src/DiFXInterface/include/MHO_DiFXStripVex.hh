#ifndef MHO_DiFXStripVex_HH__
#define MHO_DiFXStripVex_HH__

/*
*@file: MHO_DiFXStripVex.hh
*@class: MHO_DiFXStripVex
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: extracts a scan-specific portion of session vex-file (originally from RJC's d2m4 createRoot.c)
*/

namespace hops 
{

class MHO_DiFXStripVex
{
    public:
        MHO_DiFXStripVex();
        virtual ~MHO_DiFXStripVex();

        void SetSessionVexFile(std::string filename){fVexFile = filename;}
        void SetOutputFileName(std::string output_filename){fOutputFile = output_filename;}

        void ExtractScan();

    private:

        void ExtractAntenna();
        void ExtractExper();
        void ExtractGlobal();
        void ExtractMode();
        void ExtractSched();
        void ExtractSite();
        void ExtractStation();

        const char *blocks[] = {"$NO_BLOCK", "$GLOBAL", "$EXPER", "$MODE", "$STATION", "$ANTENNA",
                          "$SITE", "$BBC", "$DAS", "$FREQ", "$HEAD_POS", "$IF",
                          "$PHASE_CAL_DETECT", "$PASS_ORDER", "$PROCEDURES", "$ROLL",
                          "$SCHEDULING_PARAMS", "$SCHED", "$SEFD", "$SOURCE", "$TRACKS",
                          "$EOP", "$CLOCK", "$TAPELOG_OBS", "END_LIST"};

        enum block_tokens {NO_BLOCK, GLOBAL, EXPER, MODE, STATION, ANTENNA,
                          SITE, BBC, DAS, FREQ, HEAD_POS, IF,
                          PHASE_CAL_DETECT, PASS_ORDER, PROCEDURES, ROLL,
                          SCHEDULING_PARAMS, SCHED, SEFD, SOURCE, TRACKS,
                          EOP, CLOCK, TAPELOG_OBS, END_LIST};


};

}

#endif /* end of include guard: MHO_DiFXStripVex */