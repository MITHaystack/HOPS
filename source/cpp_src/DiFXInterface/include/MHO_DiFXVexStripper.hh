#ifndef MHO_DiFXVexStripper_HH__
#define MHO_DiFXVexStripper_HH__

/*
*@file: MHO_DiFXVexStripper.hh
*@class: MHO_DiFXVexStripper
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: extracts a scan-specific portion of session vex-file (originally from RJC's d2m4 createRoot.c)
*/

#include <string>
#include <sstream>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

class MHO_DiFXVexStripper
{
    public:
        MHO_DiFXVexStripper();
        virtual ~MHO_DiFXVexStripper();

        void SetSessionVexFile(std::string filename);
        void SetOutputFileName(std::string output_filename);
        
        void SetSourceName(std::string src_name);

        void ExtractScan();
        
    private:

        bool IsComment();
        bool IsBlockStatement();

        //modify the current vex line before output to the 'ovex' file
        void ProcessLine();

        void SpaceDelimitEqualsSign(std::string& string_to_modify);


        std::string fVexFile;
        std::string fOutputFile;
        std::string fSourceName;

        std::string fLine; //the line from the input vex file 
        std::string fOutputLine; //the modified line to the outgoing ovex file
        std::vector< std::string > fTokens;
        MHO_Tokenizer fTokenizer;
        std::string fVexDelim;
        std::string fWhitespace;

        //void ExtractAntenna();
        void ExtractExper();
        //void ExtractGlobal();
        // void ExtractMode();
        // void ExtractSched();
        // void ExtractSite();
        // void ExtractStation();

/*
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
*/

};

}


#endif /* end of include guard: MHO_DiFXVexStripper */