#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"

namespace hops
{


bool 
MHO_ManualChannelPhaseCorrectionBuilder::Build()
{
    this->fOper = new MHO_ManualChannelPhaseCorrection();
    
    // {
    //     "name": "pc_phases_x",
    //     "statement_type": "parameter",
    //     "type" : "compound",
    //     "parameters":
    //     {
    //         "channel_names": {"type": "string"},
    //         "pc_phases": {"type": "list_real"}
    //     },
    //     "fields": 
    //     [
    //         "channel_names", 
    //         "pc_phases"
    //     ]
    // }

}
// public:
// 
//     MHO_ManualChannelPhaseCorrection();
//     virtual ~MHO_ManualChannelPhaseCorrection();
// 
//     using XArgType1 = visibility_type;
//     using XArgType2 = manual_pcal_type;
//     using XArgType3 = visibility_type;
// 
//     virtual bool InitializeImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis) override;
//     virtual bool ExecuteImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis) override;
// 
// private:
    // 
    // bool fInitialized;
    // 
    // //TODO FIXME migrate these to a constants header
    // std::complex<double> fImagUnit;
    // double fDegToRad;
    // 
    // //keys for tag retrieval 
    // std::string fStationKey;
    // std::string fRemStationKey;
    // std::string fRefStationKey;
    // std::string fBaselineKey;
    // std::string fNetSidebandKey;
    // 
    // std::map< std::size_t, std::size_t> fPolIdxMap; //map pcal pol index to vis pol-product index
    // std::map< std::size_t, std::size_t> fChanIdxMap; // map pcal chan index to vis chan index
    // 
    // //minor helper function to make sure all strings are compared as upper-case only 
    // void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };
    
    
    

    

}//end namespace
