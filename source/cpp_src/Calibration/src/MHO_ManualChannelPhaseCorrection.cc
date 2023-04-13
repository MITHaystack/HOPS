#include "MHO_ManualChannelPhaseCorrection.hh"

#define PCAL_POL_AXIS 0
#define PCAL_CHANNEL_AXIS 1


namespace hops
{


MHO_ManualChannelPhaseCorrection::MHO_ManualChannelPhaseCorrection():
    fInitialized(false)
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fBaselineKey = "baseline";
    fNetSidebandKey = "net_sideband";

    fImagUnit = std::complex<double>(0.0, 1.0);
    fDegToRad = M_PI/180;
};

MHO_ManualChannelPhaseCorrection::~MHO_ManualChannelPhaseCorrection(){};


bool
MHO_ManualChannelPhaseCorrection::InitializeImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis)
{
    fInitialized = false;
    fPolIdxMap.clear();
    fChanIdxMap.clear();
    //check that dimensions of in_vis and out_vis are the same 
    if( !HaveSameDimensions(in_vis, out_vis) )
    {
        msg_warn("calibration", "manual pcal mismatch of dimensions of in/out array." << eom );
        return false;
    }

    //check that the p-cal data is tagged with a station-id that is a member of this baseline
    std::string station;
    pcal->Retrieve( fStationKey, station);

    std::string baseline;
    in_vis->Retrieve(fBaselineKey, baseline);

    if( baseline.find(station) == std::string::npos)
    {
        msg_warn("calibration", "manual pcal station: "<< station <<" not a member of baseline: "<< baseline << eom );
        return false;
    }

    //determine if the p-cal corrections are being applied to the remote or reference station
    int pol_index = 0;
    std::string rem_station;
    std::string ref_station;
    in_vis->Retrieve(fRemStationKey, rem_station);
    in_vis->Retrieve(fRefStationKey, ref_station);
    if(station != rem_station && station != ref_station)
    {
        msg_warn("calibration", "manual pcal, station: "<< station <<" not reference or remote station ("<<rem_station<<", "<<ref_station<<")."<< eom );
        return false;
    } //p-cal station, baseline mismatch

    if(station == rem_station){pol_index = 1;}
    if(station == ref_station){pol_index = 0;}

    //map the pcal polarization index to the visibility pol-product index
    auto pp_ax = std::get<POLPROD_AXIS>(*in_vis);
    auto pol_ax = std::get<PCAL_POL_AXIS>(*pcal);
    for(std::size_t j=0; j<pol_ax.GetSize(); j++)
    {
        for(std::size_t i=0; i<pp_ax.GetSize(); i++)
        {
            auto pol = pol_ax(j);
            auto polprod = pp_ax(i);
            make_upper(pol); 
            make_upper(polprod);
            if( pol[0] == polprod[pol_index] ){fPolIdxMap[i] = j;}
        }
    }

    //map the pcal channel index to the visibility channel index
    //TODO FIX/REPLACE THIS!
    auto chan_ax = std::get<CHANNEL_AXIS>(*in_vis);
    auto pcal_chan_ax = std::get<PCAL_CHANNEL_AXIS>(*pcal);
    for(std::size_t j=0; j<pcal_chan_ax.GetSize(); j++)
    {
        for(std::size_t i=0; i<chan_ax.GetSize(); i++)
        {
            std::cout<<pcal_chan_ax(j)<<", "<<chan_ax(i)<<std::endl;
            // if( pcal_chan_ax(j) == chan_ax(i) ){fChanIdxMap[i] = j;}
            if( j == i ){fChanIdxMap[i] = j;}
        }
    }

    msg_debug("calibration", "initialized manual p-cal for station: "<<station<<" in baseline: "<<baseline<<"." <<eom);

    fInitialized = true;
    return true;
}


bool
MHO_ManualChannelPhaseCorrection::ExecuteImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis)
{
    if(fInitialized)
    {
        std::cout<<"execute pcal"<<std::endl;

        for(auto it=fPolIdxMap.begin(); it !=fPolIdxMap.end(); it++)
        {
            std::cout<<"pol map: "<<it->first<<", "<<it->second<<std::endl;
        }


        for(auto it=fChanIdxMap.begin(); it !=fChanIdxMap.end(); it++)
        {
            std::cout<<"chan map: "<<it->first<<", "<<it->second<<std::endl;
        }

        //just copy in_vis into out_vis
        //TODO FIXME...there is no reason we can't do this operation in place applied to in_vs
        //but the operator interface needs to be different since we need a unary op 
        //with separate 2 input arguments (vis array, and pcal array)
        out_vis->Copy(*in_vis);

        auto chan_ax = &(std::get<CHANNEL_AXIS>(*in_vis));
    
        //loop over pol products
        for(auto pol_it = fPolIdxMap.begin(); pol_it != fPolIdxMap.end(); pol_it++)
        {
            std::size_t vis_pol_idx = pol_it->first;
            std::size_t pcal_pol_idx = pol_it->second;
            for(auto ch_it = fChanIdxMap.begin(); ch_it != fChanIdxMap.end(); ch_it++)
            {
                std::size_t vis_chan_idx = ch_it->first;
                std::size_t pcal_chan_idx = ch_it->second;
                //retrieve the p-cal phasor (assume unit normal)
                pcal_phasor_type pc_val = (*pcal)(pcal_pol_idx, pcal_chan_idx);
                std::cout<<"pc_val = " <<pc_val<<std::endl;
                visibility_element_type pc_phasor = std::exp( fImagUnit*pc_val*fDegToRad );

                //conjugate the pcal if applied to LSB
                bool do_conj = false;
                std::string sideband;
                auto labels = chan_ax->GetIntervalsWhichIntersect(vis_chan_idx);
                for(std::size_t i=0; i<labels.size(); i++)
                {
                    bool ok = labels[i]->Retrieve(fNetSidebandKey,sideband);
                    if(ok){ if(sideband == "L"){do_conj = true; break;} }
                }
                if(do_conj){pc_phasor = std::conj(pc_phasor);} 
                
                //retrieve and multiply the appropriate sub view of the visibility array 
                auto chunk = out_vis->SubView(vis_pol_idx, vis_chan_idx);
                chunk *= pc_phasor;
            }
        }
        return true;
    }
    else 
    {
        msg_warn("calibration", "manual pcal application failed, operation was not initialized. " <<eom);
        return false;
    }

};


}//end of namespace