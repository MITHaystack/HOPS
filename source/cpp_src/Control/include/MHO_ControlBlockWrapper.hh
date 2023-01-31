#ifndef MHO_ControlBlockWrapper_HH__
#define MHO_ControlBlockWrapper_HH__

#include "ffcontrol.h"

#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ContainerDefinitions.hh"

namespace hops
{

class MHO_ControlBlockWrapper 
{

    public:
        MHO_ControlBlockWrapper(c_block* block, mho_json vex_info, std::string baseline);
        virtual ~MHO_ControlBlockWrapper();


    private:

        mho_json fVexInfo;
        c_block* fControlBlock;
    
        std::string fBaseline;
        std::string fRefMk4ID;
        std::string fRemMk4ID;
        std::string fRefSiteCode;
        std::string fRemSiteCode;
        std::string fRefSiteName;
        std::string fRemSiteName;
        
        
        void Initialize();
        void DetermineStationInfo();
        void ConstructManualPhaseCalOffsets();

        // int accept_sbs[MAXFREQ];        /* accept USB, LSB, DSB iff = 1, 2, 3 */
        // short index[2*MAXFREQ];         /* index numbers of acceptable sidebands */
        // struct istats pc_mode;          /* phase cal modes */
        // struct istats pc_period;        // phase cal integration period (in ap's)
        // struct dstats pc_freq[MAXFREQ]; /* phase cal freqs (KHz) by channel */
        // struct dstats pc_phase_offset[2];// manual phase offset applied to all channels, by pol 
        // struct dstats pc_phase[MAXFREQ][2];/* phase cal phases by channel and pol 
        //                                           for manual or additive pcal */
        // struct istats pc_tonemask[MAXFREQ];// tone exclusion mask by channel in multitone
        // 
        // 
        // int nnotches;                   /* alternative to passband */
        // double notches[MAXNOTCH][2];    /* alternative to passband */
        // 
        // 
        // int nsamplers;                  // number of sampler strings
        // char *psamplers[MAX_SAMP];      // pointer to each sampler string (or NULL)
        // char sampler_codes[256];        // contains all sampler strings
        // struct dstats sampler_delay[MAX_SAMP][2]; // additive delay per sampler (s), in sampler
        // 
        // struct dstats ionosphere;       // a priori ionospheres (TEC units = 1e16 el/m^2)
        // struct dstats delay_offs[MAXFREQ];// additive delay offset(ns) by channel  ##DELAY_OFFS##
        // struct dstats delay_offs_pol[MAXFREQ][2];// additive delay offset(ns) by channel and pol
        // 
        // struct dstats pc_delay_l;       // delay diff (feed->inject)-(pulsegen->inject) (s)
        // struct dstats pc_delay_r;       // same, but for RCP (or Y or V)
        // 
        // 
        // char chid[MAXFREQ];             // single letter ch id codes for freq override
        // double chid_rf[MAXFREQ];        // freqs corresponding to above codes





 

};

}

#endif /* end of include guard: MHO_ControlBlockWrapper_HH__ */
