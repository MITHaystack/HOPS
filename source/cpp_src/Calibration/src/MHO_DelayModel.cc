#include "MHO_DelayModel.hh"
#include "MHO_Clock.hh"

#define DELAY_COEFF_INDEX 0


namespace hops 
{

MHO_DelayModel::MHO_DelayModel(){};
MHO_DelayModel::~MHO_DelayModel(){};



void 
MHO_DelayModel::compute_model()
{
    bool ok;
    std::cout<<"Clock model = "<<fClockModel.dump(2)<<std::endl;

    //fourfit reference time time point
    auto frt = hops_clock::from_vex_format(fRefTimeString);

    //get the ref/rem station codes
    std::string ref_code;
    ok = fRefData->Retrieve(std::string("station_code"), ref_code);
    if(!ok){msg_error("calibration", "station_code missing from reference station data." << eom);}
    std::string rem_code;
    ok = fRemData->Retrieve(std::string("station_code"), rem_code);
    if(!ok){msg_error("calibration", "station_code missing from remote station data." << eom);}

    //get the ref/rem station delay model start times
    std::string ref_mod_start;
    std::string rem_mod_start; 
    ok = fRefData->Retrieve(std::string("model_start"), ref_mod_start);
    if(!ok){msg_error("calibration", "model_start missing from reference station delay model data." << eom);}
    ok = fRemData->Retrieve(std::string("model_start"), rem_mod_start);
    if(!ok){msg_error("calibration", "model_start missing from remote station delay model data." << eom);}
    auto ref_start = hops_clock::from_iso8601_format(ref_mod_start);
    auto rem_start = hops_clock::from_iso8601_format(rem_mod_start);
    
    msg_debug("calibration", "fourfit reference time is: "<< hops_clock::to_iso8601_format(frt)<< eom);
    msg_debug("calibration", "reference station delay model start time is: "<<hops_clock::to_iso8601_format(ref_start)<< eom);
    msg_debug("calibration", "remote station delay model start time is: "<<hops_clock::to_iso8601_format(rem_start)<< eom);

    //calculate time differences
    auto ref_tdiff_duration = frt - ref_start;
    auto rem_tdiff_duration = frt - rem_start;

    //convert durations to double (seconds)
    double ref_tdiff = std::chrono::duration<double>(ref_tdiff_duration).count(); 
    double rem_tdiff = std::chrono::duration<double>(rem_tdiff_duration).count(); 

    double ref_model_interval;
    double rem_model_interval;
    ok = fRefData->Retrieve(std::string("model_interval"), ref_model_interval);
    if(!ok){msg_error("calibration", "model_interval missing from reference station delay model data." << eom);}
    ok = fRemData->Retrieve(std::string("model_interval"), rem_model_interval);
    if(!ok){msg_error("calibration", "model_interval missing from reference station delay model data." << eom);}

    //figure out which spline interval overlaps with the fourfit reference time
    int ref_int_no = std::floor(ref_tdiff/ref_model_interval);
    int rem_int_no = std::floor(rem_tdiff/rem_model_interval);
    CheckSplineInterval(fRefData->GetDimension(INTERVAL_AXIS), ref_tdiff, ref_int_no, ref_code);
    CheckSplineInterval(fRemData->GetDimension(INTERVAL_AXIS), rem_tdiff, rem_int_no, rem_code);

    //calculate seconds into target interval
    double ref_t = ref_tdiff - (ref_int_no * ref_model_interval);
    double rem_t = rem_tdiff - (rem_int_no * rem_model_interval);

    //evaluate delay, rate, accel
    double ref_dra[3];
    auto ref_coeff = fRefData->SubView(DELAY_COEFF_INDEX, ref_int_no); //extract spline coeffs for delay at this interval;
    EvaluateDelaySpline(ref_coeff, ref_t, ref_dra);

    double rem_dra[3];
    auto rem_coeff = fRemData->SubView(DELAY_COEFF_INDEX, rem_int_no); //extract spline coeffs for delay at this interval;
    EvaluateDelaySpline(rem_coeff, rem_t, rem_dra);

    double delay = rem_dra[0] - ref_dra[0];
    double rate = rem_dra[1] - ref_dra[1];
    double accel = rem_dra[2] - ref_dra[2];

    msg_debug("calibration", "delay model: offset, rate, accel = "<<delay<<", "<<rate<<", "<<accel<< eom);


    #pragma message("TODO: implement the reference station delay, rate, accel calculation (requires clock model), see compute_model.c")

////////////////////////////////////////////////////////////////////////////////
    // 
    // double ref_clock_rate = 0.0;
    // double ref_clock_delay = 0.0;
    // std::string ref_clock_delay_units;
    // std::string ref_clock_mod_start;
    // if(fClockModel.contains(ref_code))
    // {
    //     std::cout<<"dump clock: "<<fClockModel[ref_code].dump(2)<<std::endl;
    //     std::cout<< "rate = " << fClockModel[ref_code]["clock_early"][0]["clock_rate"]["value"] << std::endl;
    //     std::cout<< "offset = " << fClockModel[ref_code]["clock_early"][0]["clock_early_offset"]["value"] << std::endl;
    //     std::cout<< "origin epoch = " << fClockModel[ref_code]["clock_early"][0]["origin_epoch"] << std::endl;
    //     std::cout<< "start_validity_epoch = " << fClockModel[ref_code]["clock_early"][0]["start_validity_epoch"] << std::endl;
    // 
    //     ref_clock_rate = fClockModel[ref_code]["clock_early"][0]["clock_rate"]["value"];
    //     ref_clock_delay = fClockModel[ref_code]["clock_early"][0]["clock_early_offset"]["value"];
    //     ref_clock_delay_units = fClockModel[ref_code]["clock_early"][0]["clock_early_offset"]["units"];
    //     if(ref_clock_delay_units == "usec")
    //     {
    //         ref_clock_delay *= 1e-6;
    //     }
    //     ref_clock_mod_start = fClockModel[ref_code]["clock_early"][0]["origin_epoch"];
    // }
    // else 
    // {
    //     msg_warn("calibration", "reference station: "<<ref_code<<" missing from clock model."<<eom);
    // }
    // 
    // auto clock_mod_start = hops_clock::from_vex_format(ref_clock_mod_start);
    // std::cout<<"clock model start = "<<hops_clock::to_iso8601_format(clock_mod_start)<<std::endl;
    // auto ref_clock_tdiff_duration = frt - clock_mod_start;
    // double ref_clk_tdiff = std::chrono::duration<double>(ref_clock_tdiff_duration).count();
    // 
    // std::cout<<"ref_clock_delay = "<<ref_clock_delay<<std::endl;
    // 
    // //correct for clock drift
    // ref_clock_delay += ref_clk_tdiff*ref_clock_rate;
    // 
    // std::cout<<"correction for clock drift until FRT "<< ref_clk_tdiff*ref_clock_rate<<std::endl;
    // 
    //                                    /* now do calculations all over again at the
    //                                     * time of wavefront passing the ref station */
    //                                    /* Correct ref delay/rate for clocks */
    //                                    /* which are inherent in model from genaroot */
    //                                    /* Clock in usec, clockrate dimensionless */
    // // ref_delay -= t202->ref_clock * 1.0e-6;
    // // ref_rate -= t202->ref_clockrate;
    // 
    // std::cout<<"ref clk tdiff = "<<ref_clk_tdiff<<std::endl;
    // std::cout<<"ref_rate = "<<ref_clock_rate<<std::endl;
    
// 
//     // ref_delay -= ref_clk_tdiff;
//     // ref_rate -= ref_clock_rate;
// 
//                                        /* Adjust ref delay for approx time of ref */
//                                        /* station wavefront passage, not geocenter */
//                                        /* wavefront passage */
//     ref_delay *= 1.0 - ref_rate;
//                                        /* Doppler shift for ref stn, < 1 is redshift
//                                         * do this so can correct for missing or extra
//                                         * data bits in both streams (see rate calc) */
//     double ref_doppler = 1.0 - ref_rate;
//                                        /* Which model interval number? Use adjusted */
//                                        /* times */
//     ref_tdiff = ref_tdiff - ref_delay;
//     rem_tdiff = rem_tdiff - ref_delay;
// 
// 
    
}

void
MHO_DelayModel::CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id)
{
    if(tdiff < 0.0)
    {
        msg_warn("calibration", "fourfit reference time is outside of station: "<<station_id<<" spline range - must extrapolate!" << eom);
        int_no = 0;
    }
    if(int_no >= n_intervals )
    {
        msg_warn("calibration", "fourfit reference time is outside of station: "<<station_id<<" spline range - must extrapolate!" << eom);
        int_no = n_intervals-1;
    }
}




}//end namespace 
