#include "MHO_StationCoordinateModel.hh"
#include "MHO_Clock.hh"

// #define DELAY_COEFF_INDEX 0


#define AZIMUTH_COEFF_INDEX 1
#define ELEVATION_COEFF_INDEX 2
#define PARANGLE_COEFF_INDEX 3


namespace hops
{

MHO_StationCoordinateModel::MHO_StationCoordinateModel()
{
    // fDelay = 0;
    // fRate = 0;
    // fAccel = 0;
    fRefData = nullptr;
    fRemData = nullptr;
};

MHO_StationCoordinateModel::~MHO_StationCoordinateModel(){};


void
MHO_StationCoordinateModel::ComputeModel()
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
        EvaluateSpline(ref_coeff, ref_t, ref_dra);

        double rem_dra[3];
        auto rem_coeff = fRemData->SubView(DELAY_COEFF_INDEX, rem_int_no); //extract spline coeffs for delay at this interval;
        EvaluateSpline(rem_coeff, rem_t, rem_dra);

        fDelay = rem_dra[0] - ref_dra[0];
        fRate = rem_dra[1] - ref_dra[1];
        fAccel = rem_dra[2] - ref_dra[2];

        msg_debug("fringe", "delay model: offset, rate, accel = "<<fDelay<<", "<<fRate<<", "<<fAccel<< eom);
    }
    else
    {
        msg_fatal("fringe", "cannot compute delay model, missing station data. " << eom );
        std::exit(1);
    }

    #pragma message("TODO: implement the reference station delay, rate, accel calculation (requires clock model), see compute_model.c")
}

void
MHO_StationCoordinateModel::CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id)
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
