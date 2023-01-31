#include "MHO_FourfitControlInterface.hh"

namespace hops 
{

MHO_FourfitControlInterface::MHO_FourfitControlInterface()
{
    fControlFile = "";
    fControlFile = "";
    fBaseline = "";
    fSourceName = " ";
    fFrequencyGroup = "X";
    int fTime = 0;
    //this needs to be fixed...ideal by elminating the stupid global var, but otherwise via static init
    //cb_head = (struct c_block *) malloc (sizeof (struct c_block) );
    nullify_cblock(cb_head);
    default_cblock(cb_head);

    fAllocatedControlBlocks.push_back(cb_head);
};

MHO_FourfitControlInterface::~MHO_FourfitControlInterface()
{
    for(std::size_t i=0; i<fAllocatedControlBlocks.size(); i++)
    {
        free(fAllocatedControlBlocks[i]);
    }
    fAllocatedControlBlocks.clear();
};


bool
MHO_FourfitControlInterface::ConstructControlBlock()
{
    if( fControlFile != "" && 
        fBaseline.size() == 2 &&
        fFrequencyGroup.size() == 1)
    {
        //parse the control file
        struct c_block* cb_out = (struct c_block *) malloc (sizeof (struct c_block) );

        char bl[2]; 
        bl[0] = fBaseline[0]; 
        bl[1] = fBaseline[1];

        std::string src = fSourceName;
        char fgroup = fFrequencyGroup[0];

        msg_debug("ffcontrol_interface", "constructing control block from contol file: "<<fControlFile<< eom);
        msg_debug("ffcontrol_interface", "using baseline: "<<fBaseline<<", fgroup: "<<fFrequencyGroup<<", "<<" source: "<<fSourceName<<", time: "<<fTime << eom);

        int retval = construct_cblock(const_cast<char*>(fControlFile.c_str()), 
                                      cb_head, cb_out, 
                                      bl, const_cast<char*>(src.c_str()), 
                                      fgroup, fTime);
                                      
        fAllocatedControlBlocks.push_back(cb_out);
        //fCachedBlock = cb_out;
        
        std::cout<<"cbout = "<<cb_out<<std::endl;
        std::cout<<"cbout ref freq = "<<cb_out->ref_freq<<std::endl;

        if(retval == 0)
        {
            return true;
        }
        else 
        {
            msg_error("ffcontrol_interface", "control block contruction failed with error code: "<< retval << eom);
        }
    }
    else 
    {
        msg_error("ffcontrol_interface", "cannot construct control block with no control file given." << eom);
    }
    return false;
}

struct c_block* 
MHO_FourfitControlInterface::GetControlBlock()
{
    //return fCachedBlock;
    return fAllocatedControlBlocks.front();
}


}