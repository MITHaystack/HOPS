#include "MHO_ControlBlockWrapper.hh"

namespace hops 
{
    
MHO_ControlBlockWrapper::MHO_ControlBlockWrapper(c_block* block, mho_json vex_info)
{
    fControlBlock = block;
    fVexInfo = vex_info; //full copy (TODO -- maybe should just use a reference?)
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
    std::string bl;
    bl.append( &(fControlBlock->baseline[0]), 1);
    bl.append( &(fControlBlock->baseline[1]), 1);
    
    std::string ref_mk4_id; ref_mk4_id.append( &(bl[0]),1);
    std::string rem_mk4_id; rem_mk4_id.append( &(bl[1]),1);
    std::string ref_station_code = "";
    std::string rem_station_code = "";
    std::string ref_site_name = "";
    std::string rem_site_name = "";

    auto sites = fVexInfo["$SITE"];
    for(auto sit = sites.begin(); sit != sites.end(); sit++)
    {
        if( (*sit)["mk4_site_ID"] == ref_mk4_id )
        {
            ref_station_code = (*sit)["site_ID"];
            ref_site_name = (*sit)["site_name"];
        }
        
        if( (*sit)["mk4_site_ID"] == rem_mk4_id )
        {
            rem_station_code = (*sit)["site_ID"];
            rem_site_name = (*sit)["site_name"];
        }
    }


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