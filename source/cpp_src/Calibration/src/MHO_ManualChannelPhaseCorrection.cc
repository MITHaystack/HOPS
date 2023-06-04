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

    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;
};

MHO_ManualChannelPhaseCorrection::~MHO_ManualChannelPhaseCorrection(){};


bool
MHO_ManualChannelPhaseCorrection::InitializeInPlace(visibility_type* in)
{
    return false;
}
    /*
    fInitialized = false;
    fPolIdxMap.clear();
    fChanIdxMap.clear();

    //check that the p-cal data is tagged with a station-id that is a member of this baseline
    std::string station;
    pcal->Retrieve( fStationKey, station);

    //determine if the p-cal corrections are being applied to the remote or reference station
    int pol_index = 0;
    std::string rem_station;
    std::string ref_station;
    in->Retrieve(fRemStationKey, rem_station);
    in->Retrieve(fRefStationKey, ref_station);
    if(station != rem_station && station != ref_station)
    {
        msg_warn("calibration", "manual pcal, station: "<< station <<" not reference or remote station ("<<rem_station<<", "<<ref_station<<")."<< eom );
        return false;
    } //p-cal station, baseline mismatch

    if(station == rem_station){pol_index = 1;}
    if(station == ref_station){pol_index = 0;}

    //map the pcal polarization index to the visibility pol-product index
    auto pp_ax = std::get<POLPROD_AXIS>(*in);
    auto pol_ax = std::get<PCAL_POL_AXIS>(*pcal);
    for(std::size_t j=0; j<pol_ax.GetSize(); j++)
    {
        for(std::size_t i=0; i<pp_ax.GetSize(); i++)
        {
            auto pol = pol_ax(j);
            auto polprod = pp_ax(i);
            make_upper(pol);
            make_upper(polprod);
            std::cout<<"pol, polprod = "<<pol<<", "<<polprod<<std::endl;
            if( pol[0] == polprod[pol_index] ){fPolIdxMap[i] = j;}
        }
    }

    //map the pcal channel index to the visibility channel index
    //(this a pointless no-op at the moment)
    #pragma message("TODO FIX/REPLACE THE PCAL <-> VISIB CHANNEL MAP!")
    auto chan_ax = std::get<CHANNEL_AXIS>(*in);
    auto pcal_chan_ax = std::get<PCAL_CHANNEL_AXIS>(*pcal);
    for(std::size_t j=0; j<pcal_chan_ax.GetSize(); j++)
    {
        for(std::size_t i=0; i<chan_ax.GetSize(); i++)
        {
            if( j == i ){fChanIdxMap[i] = j;}
        }
    }

    msg_debug("calibration", "initialized manual p-cal for station: "<<station<<" in baseline: "<<baseline<<"." <<eom);


    fInitialized = true;
    return true;
}

*/


bool
MHO_ManualChannelPhaseCorrection::ExecuteInPlace(visibility_type* in)
{
    return false;
}
    /*
    if(fInitialized)
    {
        msg_debug("calibration", "executing manual p-cal for station. "<<eom);

        auto chan_ax = &(std::get<CHANNEL_AXIS>(*in));

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
                manual_pcal_element_type pc_val = (*pcal)(pcal_pol_idx, pcal_chan_idx);
                visibility_element_type pc_phasor = std::exp( fImagUnit*pc_val*fDegToRad );

                std::cout<<"PCAL value = "<<pc_val<<std::endl;
                pc_phasor = std::conj(pc_phasor); //conjugate for USB/LSB, but not for DSB??


                //retrieve and multiply the appropriate sub view of the visibility array
                auto chunk = in->SubView(vis_pol_idx, vis_chan_idx);
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

    return false;
}

*/


bool
MHO_ManualChannelPhaseCorrection::InitializeOutOfPlace(const visibility_type* in, visibility_type* out)
{
    return false;
    //InitializeInPlace(in);
}

bool
MHO_ManualChannelPhaseCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


}//end of namespace
