#ifndef MHO_FourfitControlInterface_HH__
#define MHO_FourfitControlInterface_HH__


//fourfit control lib
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif

#include "ffcontrol.h"
struct c_block* cb_head; //global extern kludge (due to stupid c-library interface)

#ifndef HOPS3_USE_CXX
}
#endif



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
        struct c_block* GetControlBlock();

    private:

        std::string fControlFile;
        std::string fBaseline;
        std::string fSourceName;
        std::string fFrequencyGroup;
        int fTime;
        
        struct c_block* fCachedBlock;
        std::vector< struct c_block* > fAllocatedControlBlocks;

};

}

#endif /* end of include guard: MHO_FourfitControlInterface_HH__ */
