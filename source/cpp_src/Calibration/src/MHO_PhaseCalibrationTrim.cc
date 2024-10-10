#include "MHO_PhaseCalibrationTrim.hh"
#include "MHO_SelectRepack.hh"

namespace hops
{

MHO_PhaseCalibrationTrim::MHO_PhaseCalibrationTrim()
{

};

MHO_PhaseCalibrationTrim::~MHO_PhaseCalibrationTrim(){};

bool MHO_PhaseCalibrationTrim::ExecuteInPlace(multitone_pcal_type* in)
{
    if(fVisibilities != nullptr && in != nullptr)
    {
        std::size_t vis_ntime = fVisibilities->GetDimension(TIME_AXIS);
        std::size_t pcal_ntime = in->GetDimension(MTPCAL_TIME_AXIS);
        //same number of time points, so no need to trim
        if(vis_ntime == pcal_ntime){return true;}
        else if(vis_ntime < pcal_ntime)
        {
            auto spack = new MHO_SelectRepack< multitone_pcal_type >();
        
            std::vector< std::size_t > selected_aps;
            auto vis_time_ax = &(std::get< TIME_AXIS >(*fVis));
    
            double first_t = vis_time_ax->at(0);
            double last_t = vis_time_ax->at(naps - 1);
            //TODO...convert first/last time into absolute times (w.r.t to the start time)
    

            auto pcal_time_ax = &(std::get< TIME_AXIS >(*in));
            std::size_t pcal_naps = vis_time_ax->GetSize();

    
            //Note: The stop/start parameters are passed as integers (n seconds), 
            //which works fine for 1 sec APs, but
            //for smaller APs maybe we should pass them as floats?
            for(std::size_t i = 0; i < pcal_naps; i++)
            {
                double pcal_t = (*pcal_time_ax)(i);
                //TODO convert pcal_t to absolute time
            



                //std::cout<<" t, stop, start, begin, end = "<< t <<", "<< (last_t + stop)<<", "<< (first_t - start)<<", " << first_t<<", "<<last_t<< std::endl;
                if(t <= (last_t + (double)stop) && t >= (first_t - (double)start))
                {
                    selected_aps.push_back(i);
                }
            }
            
            msg_debug("calibration", "pcal data selection, selecting " << selected_aps.size() << " APs." << eom);
            
            if(selected_aps.size() == 0)
            {
                msg_warn("calibration", "AP selection eliminated all pcal data." << eom);
            }
            
            spack->SelectAxisItems(TIME_AXIS, selected_aps);

            //not apply/execute spack

            delete spack;

            return true;
        }
    }
    else 
    {
        return false;
    }
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
