#include "MHO_ControlBlockWrapper.hh"

namespace hops 
{
    
MHO_ControlBlockWrapper::MHO_ControlBlockWrapper(c_block* block, mho_json vex_info, std::string baseline)
{
    fControlBlock = block;
    fVexInfo = vex_info; //full copy (TODO -- maybe should just use a reference?)
    fBaseline = baseline;
    if( !(fVexInfo["VEX_rev"] == "ovex") ){ msg_warn("control", "cannot find an ovex object in the vex info" << eom); }
    Initialize();
}

MHO_ControlBlockWrapper::~MHO_ControlBlockWrapper()
{
    
}



void 
MHO_ControlBlockWrapper::Initialize()
{
    DetermineStationInfo();    
    ConstructManualPhaseCalOffsets();
}

void
MHO_ControlBlockWrapper::DetermineStationInfo()
{
    fRefMk4ID = "";
    fRefMk4ID = "";
    fRefSiteCode = "";
    fRemSiteCode = "";
    fRefSiteName = "";
    fRemSiteCode = "";
    
    fRefMk4ID.append( &(fBaseline[0]),1);
    fRemMk4ID.append( &(fBaseline[1]),1);

    auto sites = fVexInfo["$SITE"];
    for(auto sit = sites.begin(); sit != sites.end(); sit++)
    {
        std::cout<<(*sit)["mk4_site_ID"]<<std::endl;
        if( (*sit)["mk4_site_ID"] == fRefMk4ID )
        {
            fRefSiteCode = (*sit)["site_ID"];
            fRefSiteName = (*sit)["site_name"];
        }
        
        if( (*sit)["mk4_site_ID"] == fRemMk4ID )
        {
            fRemSiteCode = (*sit)["site_ID"];
            fRemSiteName = (*sit)["site_name"];
        }
    }

    msg_debug("control", "control block associated with baseline: "<< fBaseline<<" with reference site: ("<< fRefMk4ID<<", "<<fRefSiteCode<<", "<<fRefSiteName<<") and remote site: ("<< fRemMk4ID<<", "<<fRemSiteCode<<", "<<fRemSiteName<<")" << eom );

}

void 
MHO_ControlBlockWrapper::ConstructManualPhaseCalOffsets()
{
    // //construct the pcal array...this is a really ugly on-off testing kludge
    // manual_pcal_type ref_pcal; ref_pcal.Resize(2,MAXFREQ);
    // manual_pcal_type rem_pcal; rem_pcal.Resize(2,MAXFREQ);
    // 
    // //label the axes
    // std::string pol_arr[2];
    // 
    // //from parser.c
    // // #define LXH 0
    // // #define RYV 1
    // 
    // pol_arr[0] = "X";
    // pol_arr[1] = "Y";
    // for(unsigned int p=0; p<2; p++)
    // {
    //     std::get<0>(ref_pcal)(p) = pol_arr[p];
    //     std::get<0>(rem_pcal)(p) = pol_arr[p];
    // }
    // 
    // for(int ch=0; ch<MAXFREQ; ch++)
    // {
    //     std::get<1>(ref_pcal)(ch) = ch;
    //     std::get<1>(rem_pcal)(ch) = ch;
    // }
    // 
    // std::complex<double> imag_unit(0.0, 1.0);
    // for(unsigned int p=0; p<2; p++)
    // {
    //     for(std::size_t ch=0; ch<MAXFREQ; ch++)
    //     {
    //         double ref_ph = cb_out->pc_phase[ch][p].ref;
    //         double rem_ph = cb_out->pc_phase[ch][p].rem;
    //         ref_pcal(p,ch) = ref_ph;// std::exp( imag_unit*2.0*M_PI*ref_ph*(M_PI/180.) );
    //         rem_pcal(p,ch) = rem_ph; //std::exp( imag_unit*2.0*M_PI*rem_ph*(M_PI/180.) );
    //         std::cout<<"chan: "<< ch <<" ref-pc: "<< cb_out->pc_phase[ch][p].ref << " rem-pc: " << cb_out->pc_phase[ch][p].rem << std::endl;
    //     }
    // }
    // 
    // ref_pcal.Insert(std::string("station"), std::string("GS") );
    // rem_pcal.Insert(std::string("station"), std::string("WF") );

    
    
}



}