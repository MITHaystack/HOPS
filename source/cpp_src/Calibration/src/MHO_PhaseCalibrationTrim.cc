#include "MHO_PhaseCalibrationTrim.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_Clock.hh"

namespace hops
{

MHO_PhaseCalibrationTrim::MHO_PhaseCalibrationTrim()
{
    fAPEps = 1e-2; //time tolerance of 0.01 sec
    fStartEps = 0.9; //allow for almost 0.9*AP
};

MHO_PhaseCalibrationTrim::~MHO_PhaseCalibrationTrim(){};

bool MHO_PhaseCalibrationTrim::ExecuteInPlace(multitone_pcal_type* in)
{
    if(fVis != nullptr && in != nullptr)
    {
        //get the start times tags of the vis and pcal data 
        std::string vis_start;
        std::string pcal_start;

        bool vis_start_ok = fVis->Retrieve("start", vis_start);
        bool pcal_start_ok = in->Retrieve("start", pcal_start);

        msg_debug("calibration", "pcal start time is: "<< pcal_start << eom);
        msg_debug("calibration", "visibility start time is: "<< vis_start << eom);

        if(!vis_start_ok)
        {
            msg_error("calibration", "no start time specified for visbility data, cannot trim pcal data" << eom);
            return false;
        }

        if(!pcal_start_ok)
        {
            msg_warn("calibration", "no start time specified for pcal data, assuming it is the same as the visibilities" << eom);
            pcal_start = vis_start;
        }

        auto vis_time_ax = &(std::get< TIME_AXIS >(*fVis));
        auto pcal_time_ax = &(std::get<MTPCAL_TIME_AXIS>(*in));
        std::size_t vis_ntime =  vis_time_ax->GetSize();
        std::size_t pcal_ntime = pcal_time_ax->GetSize();

        if(vis_ntime < 2 || pcal_ntime < 2)
        {
            msg_error("calibration", "too few points on time axis of visibility or pcal data" << eom );
            return false;
        }

        //figure out the size of the steps on the time axis 
        double vis_tdelta = vis_time_ax->at(1) - vis_time_ax->at(0);
        double pcal_tdelta = pcal_time_ax->at(1) - pcal_time_ax->at(0);
        double ap_delta = std::fabs( vis_tdelta - pcal_tdelta);
        if( ap_delta > fAPEps )
        {
            msg_error("calibration", "pcal accumulation period and visibility accumulation period are different by: "<< ap_delta << eom );
            return false;
        }

        //now figure out the start time delta, and the number of points on time axis
        auto vis_start_tp = hops_clock::from_vex_format(vis_start);
        auto pcal_start_tp = hops_clock::from_vex_format(pcal_start);

        //calculate time differences
        auto start_tdiff_duration = pcal_start_tp - vis_start_tp;
        //convert difference to double (seconds) ... we subtract of pcal_tdelta/2 because pcal data uses time centroids
        double start_tdiff = std::chrono::duration< double >(start_tdiff_duration).count() - pcal_tdelta/2.0;

        if( std::fabs(start_tdiff) < fStartEps*vis_tdelta && vis_ntime == pcal_ntime )
        {
            msg_debug("calibration", "pcal accumulation period and start time coincide within tolerance, no need to trim pcal data" << eom);
            msg_debug("calibration", "start time delta is: "<< start_tdiff << 
                ", num pcal points: "<<pcal_ntime<<", num vis points: "<< vis_ntime << eom );
            return true;
        }
        else 
        {
            msg_debug("calibration", "need to trim pcal data to match visibilities, start time delta is: "<< start_tdiff << 
                ", num pcal points: "<<pcal_ntime<<", num vis points: "<< vis_ntime << eom );
        }

        //evidently the start time's are different enough, or the number of points are different, so we need to trim down 
        //the pcal data so that it matches the same time period as the visibility data
        //we use select-repack to do this, and select the ap indices we want to keep by comparing absolute times
        double vis_first_t = vis_time_ax->at(0);
        double vis_last_t = vis_time_ax->at(vis_ntime - 1) + vis_tdelta;
        int64_t vis_first_t_ns = 1e9*vis_first_t;
        int64_t vis_last_t_ns = 1e9*vis_last_t;
        auto vis_first_tp = vis_start_tp + hops_clock::duration(vis_first_t_ns);
        auto vis_last_tp = vis_start_tp + hops_clock::duration(vis_last_t_ns);

        msg_debug("calibration", "selecting pcal data between "<<hops_clock::to_vex_format(vis_first_tp)<<" and "<<
            hops_clock::to_vex_format(vis_last_tp) << eom );

        auto spack = new MHO_SelectRepack< multitone_pcal_type >();
        std::vector< std::size_t > selected_aps;
        for(std::size_t i = 0; i < pcal_ntime; i++)
        {
            double pcal_t = pcal_time_ax->at(i);
            int64_t pcal_t_ns = 1e9*pcal_t;
            auto pcal_tp = pcal_start_tp + hops_clock::duration(pcal_t_ns);

            if( pcal_tp <= vis_last_tp && pcal_tp >= vis_first_tp )
            {
                selected_aps.push_back(i);
            }
        }
        
        msg_debug("calibration", "pcal data selection, selecting " << selected_aps.size() << " APs" << eom);
        if(selected_aps.size() == 0)
        {
            msg_warn("calibration", "AP selection eliminated all pcal data." << eom);
        }

        //now apply/execute spack
        spack->SetArgs(in);
        spack->SelectAxisItems(MTPCAL_TIME_AXIS, selected_aps);
        bool ok = spack->Initialize();
        ok = spack->Execute();

        //the new start time
        int64_t pcal_t_ns = 1e9*pcal_time_ax->at(0);
        pcal_start_tp += hops_clock::duration(pcal_t_ns);
        std::string new_pcal_start = hops_clock::to_vex_format(pcal_start_tp);
        //in->Insert("start", new_pcal_start);
    
        msg_debug("calibration", "trimmed pcal, original start: " << pcal_start <<", new start: "<< new_pcal_start << eom );
        msg_debug("calibration", "trimmed pcal, original number of APs: "<<pcal_ntime<<", new number of APs: "<< in->GetDimension(MTPCAL_TIME_AXIS) << eom);

        delete spack;

        return ok;
    }
    return false;
}

bool MHO_PhaseCalibrationTrim::ExecuteOutOfPlace(const multitone_pcal_type* in, multitone_pcal_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

bool MHO_PhaseCalibrationTrim::InitializeInPlace(multitone_pcal_type* /*in*/)
{
    return true;
}

bool MHO_PhaseCalibrationTrim::InitializeOutOfPlace(const multitone_pcal_type* /*in*/, multitone_pcal_type* /*out*/)
{
    return true;
}

} // namespace hops
