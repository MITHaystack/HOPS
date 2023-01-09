#include "MHO_ManualChannelPhaseCorrection.hh"

namespace hops
{


MHO_ManualChannelPhaseCorrection::MHO_ManualChannelPhaseCorrection():
    fInitialized(false),
    fConjugate(false)
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fBaselineKey = "baseline";
};

MHO_ManualChannelPhaseCorrection::~MHO_ManualChannelPhaseCorrection(){};


bool
MHO_ManualChannelPhaseCorrection::InitializeImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis)
{
    fInitialized = false;
    fPolIdxMap.clear();
    fChanIdxMap.clear();
    //check that dimensions of in_vis and out_vis are the same 
    if( !HaveSameDimensions(in_vis, out_vis) ){return false;}

    //check that the p-cal data is tagged with a station-id that is a member of this baseline
    std::string station;
    pcal->Retrieve( fStationKey, station);

    std::cout<<"pcal station = "<<station<<std::endl;

    std::string baseline;
    in_vis->Retrieve(fBaselineKey, baseline);

    std::cout<<"baseline = "<<baseline<<std::endl;

    if( baseline.find(station) == std::string::npos){return false;}

    //determine if the p-cal corrections are being applied to the remote or reference station
    int pol_index = 0;
    std::string rem_station;
    std::string ref_station;
    in_vis->Retrieve(fRemStationKey, rem_station);
    in_vis->Retrieve(fRefStationKey, ref_station);
    if(station != rem_station && station != ref_station){return false;} //p-cal station, baseline mismatch
    if(station == rem_station){fConjugate = true; pol_index = 1;}
    if(station == ref_station){fConjugate = false; pol_index = 0;}

    if(fConjugate){std::cout<<"is remote station"<<std::endl;}

    //map the pcal polarization index to the visibility pol-product index
    auto pp_ax = std::get<CH_POLPROD_AXIS>(*in_vis);
    auto pol_ax = std::get<0>(*pcal);
    for(std::size_t j=0; j<pol_ax.GetSize(); j++)
    {
        for(std::size_t i=0; i<pp_ax.GetSize(); i++)
        {
            auto pol = pol_ax(j);
            auto polprod = pp_ax(i);

            std::cout<<"pol_axis = "<<pol<<" polprod_ax = "<<polprod<<std::endl;
            if( pol[0] == polprod[pol_index] ){fPolIdxMap[i] = j; std::cout<<i<<" -> "<<j<<std::endl;}
            //if( pol_ax(i)[0] == pp_ax(i)[pol_index] ){fPolIdxMap[j] = i;}
        }
    }

    //map the pcal channel index to the visibility channel index
    auto chan_ax = std::get<CH_CHANNEL_AXIS>(*in_vis);
    auto pcal_chan_ax = std::get<1>(*pcal);
    for(std::size_t j=0; j<pcal_chan_ax.GetSize(); j++)
    {
        for(std::size_t i=0; i<chan_ax.GetSize(); i++)
        {
            if( pcal_chan_ax(j) == chan_ax(i) ){fChanIdxMap[i] = j;}
        }
    }

    std::cout<<"init done"<<std::endl;

    fInitialized = true;
    return true;
}


bool
MHO_ManualChannelPhaseCorrection::ExecuteImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis)
{
    if(fInitialized)
    {
        //just copy in_vis into out_vis
        //TODO FIXME...there is no reason we can't do this operation in place applied to in_vs
        //but the operator interface needs to be different since we need a unary op 
        //with separate 2 input arguments (vis array, and pcal array)
        out_vis->Copy(*in_vis);
    
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
                //conjugate if applied to remote station
                if(fConjugate){pc_val = std::conj(pc_val);} 
                
                //retrieve and multiply the appropriate sub view of the visibility array 
                auto chunk = out_vis->SubView(vis_pol_idx, vis_chan_idx);
                chunk *= pc_val;
            }
        }
    }

    return true;
};


}//end of namespace