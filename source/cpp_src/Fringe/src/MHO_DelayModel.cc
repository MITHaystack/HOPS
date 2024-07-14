#include "MHO_DelayModel.hh"
#include "MHO_Clock.hh"

#define DELAY_COEFF_INDEX 0

namespace hops
{

MHO_DelayModel::MHO_DelayModel()
{
    fDelay = 0;
    fRate = 0;
    fAccel = 0;
    fRefData = nullptr;
    fRemData = nullptr;
};

MHO_DelayModel::~MHO_DelayModel(){};


void
MHO_DelayModel::ComputeModel()
{
    if(fRefData !=nullptr && fRemData != nullptr)
    {
        //convert fourfit reference time string to time point
        auto frt = hops_clock::from_vex_format(fRefTimeString);

        //get the ref/rem station codes
        std::string ref_code = RetrieveTag<std::string>(fRefData, "station_code");
        std::string rem_code = RetrieveTag<std::string>(fRemData, "station_code");

        msg_debug("fringe", "reference staion code: " << ref_code << eom );
        msg_debug("fringe", "remote staion code: " << rem_code << eom );

        //get the ref/rem station delay model start times
        std::string ref_mod_start = RetrieveTag<std::string>(fRefData, "model_start");
        std::string rem_mod_start = RetrieveTag<std::string>(fRemData, "model_start");
        //convert string to time point
        auto ref_start = hops_clock::from_vex_format(ref_mod_start);
        auto rem_start = hops_clock::from_vex_format(rem_mod_start);

        msg_debug("fringe", "fourfit reference time is: "<< hops_clock::to_iso8601_format(frt)<< eom);
        msg_debug("fringe", "reference station delay model start time is: "<<hops_clock::to_iso8601_format(ref_start)<< eom);
        msg_debug("fringe", "remote station delay model start time is: "<<hops_clock::to_iso8601_format(rem_start)<< eom);

        //calculate time differences
        auto ref_tdiff_duration = frt - ref_start;
        auto rem_tdiff_duration = frt - rem_start;

        //convert durations to double (seconds)
        double ref_tdiff = std::chrono::duration<double>(ref_tdiff_duration).count();
        double rem_tdiff = std::chrono::duration<double>(rem_tdiff_duration).count();

        double ref_model_interval = RetrieveTag<double>(fRefData, "model_interval");
        double rem_model_interval = RetrieveTag<double>(fRemData, "model_interval");

        //figure out which spline interval overlaps with the fourfit reference time
        int ref_int_no = std::floor(ref_tdiff/ref_model_interval);
        int rem_int_no = std::floor(rem_tdiff/rem_model_interval);
        CheckSplineInterval(fRefData->GetDimension(INTERVAL_AXIS), ref_tdiff, ref_int_no, ref_code);
        CheckSplineInterval(fRemData->GetDimension(INTERVAL_AXIS), rem_tdiff, rem_int_no, rem_code);

        //calculate seconds into target interval
        double ref_t = ref_tdiff - (ref_int_no * ref_model_interval);
        double rem_t = rem_tdiff - (rem_int_no * rem_model_interval);

        msg_debug("fringe", "delay_model: ref model interval: "<< ref_int_no <<" and time offset: "<< ref_t << eom);
        msg_debug("fringe", "delay_model: rem model interval: "<< rem_int_no <<" and time offset: "<< rem_t << eom);

        //evaluate delay, rate, accel
        double ref_dra[3];
        auto ref_coeff = fRefData->SubView(DELAY_COEFF_INDEX, ref_int_no); //extract spline coeffs for delay at this interval;
        EvaluateDelaySpline(ref_coeff, ref_t, ref_dra);

        double rem_dra[3];
        auto rem_coeff = fRemData->SubView(DELAY_COEFF_INDEX, rem_int_no); //extract spline coeffs for delay at this interval;
        EvaluateDelaySpline(rem_coeff, rem_t, rem_dra);

        fDelay = rem_dra[0] - ref_dra[0];
        fRate = rem_dra[1] - ref_dra[1];
        fAccel = rem_dra[2] - ref_dra[2];

        msg_debug("fringe", "delay model: offset, rate, accel = "<<fDelay<<", "<<fRate<<", "<<fAccel<< eom);

        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        //The following section calculates the a priori delay and rate at the
        //reference station. This is not actually used anywhere by the rest of the
        //fringe algorithm, and is only used for populating the type_208s on export.
        //It should be a candidate for deprecation if the 208 functionality is unneeded.



        /* now do calculations all over again at the
         * time of wavefront passing the ref station */
        /* Correct ref delay/rate for clocks */
        /* which are inherent in model from genaroot */
        /* Clock in usec, clockrate dimensionless */
        ref_dra[0] -= fRefClockOff;
        ref_dra[1] -= fRefClockRate;
        /* Adjust ref delay for approx time of ref */
        /* station wavefront passage, not geocenter */
        /* wavefront passage */
        ref_dra[0] *= 1.0 - ref_dra[1];
        /* Doppler shift for ref stn, < 1 is redshift
         * do this so can correct for missing or extra
         * data bits in both streams (see rate calc) */
        double ref_doppler = 1.0 - ref_dra[1];
        /* Which model interval number? Use adjusted */
        /* times */

        //re-calculate time differences
        ref_tdiff_duration = frt - ref_start;
        rem_tdiff_duration = frt - rem_start;
        //convert durations to double (seconds)
        ref_tdiff = std::chrono::duration<double>(ref_tdiff_duration).count() - ref_dra[0];
        rem_tdiff = std::chrono::duration<double>(rem_tdiff_duration).count() - ref_dra[0];
        //re-calculate which spline interval we are on
        ref_int_no = std::floor(ref_tdiff/ref_model_interval);
        rem_int_no = std::floor(rem_tdiff/rem_model_interval);
        CheckSplineInterval(fRefData->GetDimension(INTERVAL_AXIS), ref_tdiff, ref_int_no, ref_code);
        CheckSplineInterval(fRemData->GetDimension(INTERVAL_AXIS), rem_tdiff, rem_int_no, rem_code);

        //re-calculate seconds into target interval
        ref_t = ref_tdiff - (ref_int_no * ref_model_interval);
        rem_t = rem_tdiff - (rem_int_no * rem_model_interval);

        //evaluate delay, rate, accel
        std::cout<<"ref int no = "<<ref_int_no<<std::endl;
        ref_coeff = fRefData->SubView(DELAY_COEFF_INDEX, ref_int_no); //extract spline coeffs for delay at this interval;
        EvaluateDelaySpline(ref_coeff, ref_t, ref_dra);

        rem_coeff = fRemData->SubView(DELAY_COEFF_INDEX, rem_int_no); //extract spline coeffs for delay at this interval;
        EvaluateDelaySpline(rem_coeff, rem_t, rem_dra);

        fRefDelay = rem_dra[0] - ref_dra[0];
        fRefRate  = (rem_dra[1] - ref_dra[1]) * ref_doppler;
        fRefStationDelay = ref_dra[0];

    }
    else
    {
        msg_fatal("fringe", "cannot compute delay model, missing station data. " << eom );
        std::exit(1);
    }

}


void
MHO_DelayModel::CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id)
{
    if(n_intervals == 0)
    {
        msg_fatal("fringe", "number of spline intervals is 0, missing or malformed data?" << eom );
        std::exit(1);
    }

    if(tdiff < 0.0)
    {
        msg_warn("fringe", "fourfit reference time is outside of station: "<<station_id<<" spline range - must extrapolate!" << eom);
        int_no = 0;
    }
    if(int_no >= n_intervals )
    {
        msg_warn("fringe", "fourfit reference time is outside of station: "<<station_id<<" spline range - must extrapolate!" << eom);
        int_no = n_intervals-1;
    }
}


}//end namespace
