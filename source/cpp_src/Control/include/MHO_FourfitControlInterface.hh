#ifndef MHO_FourfitControlInterface_HH__
#define MHO_FourfitControlInterface_HH__

#include "ffcontrol.h"

#include <string>
#include <vector>

#include "MHO_Message.hh"



namespace hops 
{

class MHO_FourfitControlInterface
{
    public:

        MHO_FourfitControlInterface();
        virtual ~MHO_FourfitControlInterface();

        void SetControlFile(std::string control_file){fControlFile = control_file;}

        void SetBaseline(std::string baseline){fBaseline = baseline;};
        void SetSourceName(std::string src_name){fSourceName = src_name;};
        void SetFrequencyGroup(std::string freq_group){fFrequencyGroup = freq_group;};
        void SetTime(int time_val){fTime = time_val;};

        //returns true if sucessful, false if error
        bool ConstructControlBlock();
        c_block* GetControlBlock();

    private:

        std::string fControlFile;
        std::string fBaseline;
        std::string fSourceName;
        std::string fFrequencyGroup;
        int fTime;

        std::vector< c_block* > fAllocatedControlBlocks;

};

}

#endif /* end of include guard: MHO_FourfitControlInterface_HH__ */
