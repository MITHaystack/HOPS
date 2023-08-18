#include "MHO_DelayModel.hh"

#include "MHO_Clock.hh"

namespace hops 
{

MHO_DelayModel::MHO_DelayModel(){};
MHO_DelayModel::~MHO_DelayModel(){};



void 
MHO_DelayModel::compute_model()
{
    bool ok;
    std::cout<<"Clock model = "<<fClockModel.dump(2)<<std::endl;


    //get the ref/rem station codes
    std::string ref_code;
    ok = fRefData->Retrieve(std::string("station_code"), ref_code);
    if(!ok){msg_error("calibration", "station_code missing from reference station data." << eom);}

    // std::string rem_code;
    // ok = fRemData->Retrieve(std::string("station_code"), rem_code);
    // if(!ok){msg_error("calibration", "station_code missing from remote station data." << eom);}

    double ref_clock_rate = 0.0;
    double ref_clock_delay = 0.0;
    std::string ref_clock_mod_start;
    if(fClockModel.contains(ref_code))
    {
        std::cout<<"dump clock: "<<fClockModel[ref_code].dump(2)<<std::endl;
        std::cout<< "rate = " << fClockModel[ref_code]["clock_early"][0]["clock_rate"]["value"] << std::endl;
        std::cout<< "offset = " << fClockModel[ref_code]["clock_early"][0]["clock_early_offset"]["value"] << std::endl;
        std::cout<< "origin epoch = " << fClockModel[ref_code]["clock_early"][0]["origin_epoch"] << std::endl;
        std::cout<< "start_validity_epoch = " << fClockModel[ref_code]["clock_early"][0]["start_validity_epoch"] << std::endl;

        ref_clock_rate = fClockModel[ref_code]["clock_early"][0]["clock_rate"]["value"];
        ref_clock_delay = fClockModel[ref_code]["clock_early"][0]["clock_early_offset"]["value"];
        //ref_clock_delay *= 1e-6;
        ref_clock_mod_start = fClockModel[ref_code]["clock_early"][0]["origin_epoch"];
    }
    else 
    {
        msg_warn("calibration", "reference station: "<<ref_code<<" missing from clock model."<<eom);
    }

    // int i;
    // double ref_mod_start, rem_mod_start, ref_tdiff, rem_tdiff;
    // double ref_int_no, ref_t, ref_t2, ref_t3, ref_t4, ref_t5;
    // double rem_int_no, rem_t, rem_t2, rem_t3, rem_t4, rem_t5;
    // double ref_delay, ref_rate, ref_accel;
    // double rem_delay, rem_rate, rem_accel;
    // double ref_doppler;
    // struct mk4_sdata *refsd, *remsd;
    // struct type_301 *ref301, *rem301;

    //double reftime;
    //double ref_mod_start, rem_mod_start;
    std::string ref_mod_start;
    std::string rem_mod_start; 


    ok = fRefData->Retrieve(std::string("model_start"), ref_mod_start);
    if(!ok){msg_error("calibration", "model_start missing from reference station delay model data." << eom);}
    ok = fRemData->Retrieve(std::string("model_start"), rem_mod_start);
    if(!ok){msg_error("calibration", "model_start missing from remote station delay model data." << eom);}

    auto frt = hops_clock::from_vex_format(fRefTimeString);
    auto clock_mod_start = hops_clock::from_vex_format(ref_clock_mod_start);
    auto ref_start = hops_clock::from_iso8601_format(ref_mod_start);
    auto rem_start = hops_clock::from_iso8601_format(rem_mod_start);
    
    auto ref_clock_tdiff_duration = frt - clock_mod_start;
    auto ref_tdiff_duration = frt - ref_start;
    auto rem_tdiff_duration = frt - rem_start;

    //convert to double (seconds)
    double ref_tdiff = std::chrono::duration<double>(ref_tdiff_duration).count(); 
    double rem_tdiff = std::chrono::duration<double>(rem_tdiff_duration).count(); 

    double ref_clk_tdiff = std::chrono::duration<double>(ref_clock_tdiff_duration).count();

    //correct for clock drift
    ref_clock_delay += ref_clk_tdiff*ref_clock_rate;


    std::cout<<hops_clock::to_iso8601_format(frt)<<std::endl;
    std::cout<<hops_clock::to_iso8601_format(ref_start)<<std::endl;
    std::cout<<hops_clock::to_iso8601_format(rem_start)<<std::endl;

    std::cout<<ref_tdiff<<std::endl;
    std::cout<<rem_tdiff<<std::endl;

    double ref_model_interval;
    double rem_model_interval;
    ok = fRefData->Retrieve(std::string("model_interval"), ref_model_interval);
    if(!ok){msg_error("calibration", "model_interval missing from reference station delay model data." << eom);}
    ok = fRemData->Retrieve(std::string("model_interval"), rem_model_interval);
    if(!ok){msg_error("calibration", "model_interval missing from reference station delay model data." << eom);}

    int ref_int_no = std::floor(ref_tdiff/ref_model_interval);
    int rem_int_no = std::floor(rem_tdiff/rem_model_interval);

    if(ref_tdiff < 0.0)
    {
        msg_warn("calibration", "FRT is outside of reference station spline range - must extrapolate!" << eom);
        ref_int_no = 0;
    }

    if(rem_tdiff < 0.0)
    {
        msg_warn("calibration", "FRT is outside of remote station spline range - must extrapolate!" << eom);
        rem_int_no = 0;
    }   

    if(ref_int_no >= fRefData->GetDimension(INTERVAL_AXIS) )
    {
        msg_warn("calibration", "FRT is outside of ref spline range - must extrapolate!" << eom);
        ref_int_no = fRefData->GetDimension(0)-1;
    }

    if(rem_int_no >= fRemData->GetDimension(INTERVAL_AXIS) )
    {
        msg_warn("calibration", "FRT is outside of ref spline range - must extrapolate!" << eom);
        rem_int_no = fRemData->GetDimension(0)-1;
    }

    //seconds in target interval
    double ref_t = ref_tdiff - (ref_int_no * ref_model_interval);
    double rem_t = rem_tdiff - (rem_int_no * rem_model_interval);

    //compute delays
    double ref_delay = 0.0;
    double rem_delay = 0.0;
    double ref_rate = 0.0;
    double rem_rate = 0.0;
    double ref_accel = 0.0;
    double rem_accel = 0.0;
    int n_coeff = fRefData->GetDimension(COEFF_AXIS);
    for(int p=0; p<n_coeff; p++)
    {
        double ref_tp = std::pow(ref_t, p);
        double rem_tp = std::pow(rem_t, p);
        double ref_tpm1 = std::pow(ref_t, p-1);
        double rem_tpm1 = std::pow(rem_t, p-1);
        double ref_tpm2 = std::pow(ref_t, p-2);
        double rem_tpm2 = std::pow(rem_t, p-2);
        ref_delay += fRefData->at(0, ref_int_no, p) * ref_tp;
        rem_delay += fRemData->at(0, rem_int_no, p) * rem_tp;
        ref_rate += fRefData->at(0, ref_int_no, p) * p * ref_tpm1;
        rem_rate += fRemData->at(0, rem_int_no, p) * p * rem_tpm1;
        ref_accel += fRefData->at(0, ref_int_no, p) * p * (p-1) * ref_tpm2;
        rem_accel += fRemData->at(0, rem_int_no, p) * p * (p-1) * rem_tpm2;
    }
    
    std::cout<<"ref_delay = "<<ref_delay<<std::endl;
    std::cout<<"rem_delay = "<<rem_delay<<std::endl;
    std::cout<<"ref_rate = "<<ref_rate<<std::endl;
    std::cout<<"rem_rate = "<<rem_rate<<std::endl;
    std::cout<<"ref_accel = "<<ref_accel<<std::endl;
    std::cout<<"rem_accel = "<<rem_accel<<std::endl;

    double delay = rem_delay - ref_delay;
    double rate = rem_rate - ref_rate;
    double accel = rem_accel - ref_accel;

    std::cout<<"delay, rate, accel= "<<delay<<", "<<rate<<", "<<accel<<std::endl;


////////////////////////////////////////////////////////////////////////////////

    
                                       /* now do calculations all over again at the
                                        * time of wavefront passing the ref station */
                                       /* Correct ref delay/rate for clocks */
                                       /* which are inherent in model from genaroot */
                                       /* Clock in usec, clockrate dimensionless */
    // ref_delay -= t202->ref_clock * 1.0e-6;
    // ref_rate -= t202->ref_clockrate;

    std::cout<<"ref clk tdiff = "<<ref_clk_tdiff<<std::endl;
    std::cout<<"ref_rate = "<<ref_clock_rate<<std::endl;

    // ref_delay -= ref_clk_tdiff;
    // ref_rate -= ref_clock_rate;

                                       /* Adjust ref delay for approx time of ref */
                                       /* station wavefront passage, not geocenter */
                                       /* wavefront passage */
    ref_delay *= 1.0 - ref_rate;
                                       /* Doppler shift for ref stn, < 1 is redshift
                                        * do this so can correct for missing or extra
                                        * data bits in both streams (see rate calc) */
    double ref_doppler = 1.0 - ref_rate;
                                       /* Which model interval number? Use adjusted */
                                       /* times */
    ref_tdiff = ref_tdiff - ref_delay;
    rem_tdiff = rem_tdiff - ref_delay;


////////////////////////////////////////////////////////////////////////////////


    ref_int_no = std::floor(ref_tdiff/ref_model_interval);
    rem_int_no = std::floor(rem_tdiff/rem_model_interval);

    if(ref_tdiff < 0.0)
    {
        msg_warn("calibration", "FRT is outside of reference station spline range - must extrapolate!" << eom);
        ref_int_no = 0;
    }

    if(rem_tdiff < 0.0)
    {
        msg_warn("calibration", "FRT is outside of remote station spline range - must extrapolate!" << eom);
        rem_int_no = 0;
    }   

    if(ref_int_no >= fRefData->GetDimension(INTERVAL_AXIS) )
    {
        msg_warn("calibration", "FRT is outside of ref spline range - must extrapolate!" << eom);
        ref_int_no = fRefData->GetDimension(0)-1;
    }

    if(rem_int_no >= fRemData->GetDimension(INTERVAL_AXIS) )
    {
        msg_warn("calibration", "FRT is outside of ref spline range - must extrapolate!" << eom);
        rem_int_no = fRemData->GetDimension(0)-1;
    }

    //seconds in target interval
    ref_t = ref_tdiff - (ref_int_no * ref_model_interval);
    rem_t = rem_tdiff - (rem_int_no * rem_model_interval);

    //compute delays
    ref_delay = 0.0;
    rem_delay = 0.0;
    ref_rate = 0.0;
    rem_rate = 0.0;
    ref_accel = 0.0;
    rem_accel = 0.0;
    n_coeff = fRefData->GetDimension(COEFF_AXIS);
    for(int p=0; p<n_coeff; p++)
    {
        double ref_tp = std::pow(ref_t, p);
        double rem_tp = std::pow(rem_t, p);
        double ref_tpm1 = std::pow(ref_t, p-1);
        double rem_tpm1 = std::pow(rem_t, p-1);
        double ref_tpm2 = std::pow(ref_t, p-2);
        double rem_tpm2 = std::pow(rem_t, p-2);
        ref_delay += fRefData->at(0, ref_int_no, p) * ref_tp;
        rem_delay += fRemData->at(0, rem_int_no, p) * rem_tp;
        ref_rate += fRefData->at(0, ref_int_no, p) * p * ref_tpm1;
        rem_rate += fRemData->at(0, rem_int_no, p) * p * rem_tpm1;
        ref_accel += fRefData->at(0, ref_int_no, p) * p * (p-1) * ref_tpm2;
        rem_accel += fRemData->at(0, rem_int_no, p) * p * (p-1) * rem_tpm2;
    }

    std::cout<<"ref_delay = "<<ref_delay<<std::endl;
    std::cout<<"rem_delay = "<<rem_delay<<std::endl;
    std::cout<<"ref_rate = "<<ref_rate<<std::endl;
    std::cout<<"rem_rate = "<<rem_rate<<std::endl;
    std::cout<<"ref_accel = "<<ref_accel<<std::endl;
    std::cout<<"rem_accel = "<<rem_accel<<std::endl;

    double delay_ref = rem_delay - ref_delay;
    double rate_ref  = (rem_rate - ref_rate) * ref_doppler;
    double ref_stn_delay = ref_delay;
    

    std::cout<<"delay_ref, rate_ref, ref_stn_delay"<<delay_ref<<", "<<rate_ref<<", "<<ref_stn_delay<<std::endl;















    // ref_int_no = floor (ref_tdiff / (double)refsd->t300->model_interval);
    // rem_int_no = floor (rem_tdiff / (double)remsd->t300->model_interval);
    //                                     /* Locate corresponding type 301 records */
    //                                     /* The delay splines should be the same */
    //                                     /* for all channels, so we just take */
    //                                     /* channel 0 */
    // for (i=0; i<refsd->t300->nsplines; i++)
    //     if (refsd->model[0].t301[i]->interval == (int)ref_int_no) 
    //         break;
    // if (i == refsd->t300->nsplines)
    //     {
    //     msg ("Warning!! FRT is outside of ref spline range - must extrapolate!", 2);
    //                                 // use spline nearest the frt
    //     if (ref_tdiff < 0.0)
    //         i = 0;
    //     else
    //         i = refsd->t300->nsplines - 1;
    //     }
    // ref301 = refsd->model[0].t301[i];
    // 
    // for (i=0; i<remsd->t300->nsplines; i++)
    //     if (remsd->model[0].t301[i]->interval == (int)rem_int_no)
    //         break;
    // if (i == remsd->t300->nsplines)
    //     {
    //     msg ("Warning!! FRT is outside of rem spline range - must extrapolate!", 2);
    //                                 // use spline nearest the frt
    //     if (rem_tdiff < 0.0)
    //         i = 0;
    //     else
    //         i = remsd->t300->nsplines - 1;
    //     }
    // rem301 = remsd->model[0].t301[i];
    //                                     /* Seconds in target interval */
    // ref_t = ref_tdiff - (ref_int_no * refsd->t300->model_interval);
    // rem_t = rem_tdiff - (rem_int_no * remsd->t300->model_interval);
    //                                     /* Powers of t */
    // ref_t2 = ref_t * ref_t;
    // ref_t3 = ref_t2 * ref_t;
    // ref_t4 = ref_t3 * ref_t;
    // ref_t5 = ref_t4 * ref_t;
    // rem_t2 = rem_t * rem_t;
    // rem_t3 = rem_t2 * rem_t;
    // rem_t4 = rem_t3 * rem_t;
    // rem_t5 = rem_t4 * rem_t;
    //                                     /* Compute delays */
    // ref_delay = ref301->delay_spline[0]
    //                 + ref301->delay_spline[1] * ref_t
    //                 + ref301->delay_spline[2] * ref_t2
    //                 + ref301->delay_spline[3] * ref_t3
    //                 + ref301->delay_spline[4] * ref_t4
    //                 + ref301->delay_spline[5] * ref_t5;
    // rem_delay = rem301->delay_spline[0]
    //                 + rem301->delay_spline[1] * rem_t
    //                 + rem301->delay_spline[2] * rem_t2
    //                 + rem301->delay_spline[3] * rem_t3
    //                 + rem301->delay_spline[4] * rem_t4
    //                 + rem301->delay_spline[5] * rem_t5;
    //                                     /* Compute delay rates */
    // ref_rate = ref301->delay_spline[1]
    //                 + ref301->delay_spline[2] * 2.0 * ref_t
    //                 + ref301->delay_spline[3] * 3.0 * ref_t2
    //                 + ref301->delay_spline[4] * 4.0 * ref_t3
    //                 + ref301->delay_spline[5] * 5.0 * ref_t4;
    // rem_rate = rem301->delay_spline[1]
    //                 + rem301->delay_spline[2] * 2.0 * rem_t
    //                 + rem301->delay_spline[3] * 3.0 * rem_t2
    //                 + rem301->delay_spline[4] * 4.0 * rem_t3
    //                 + rem301->delay_spline[5] * 5.0 * rem_t4;
    // 
    //                                     /* Baseline apriori model */
    // msg ("refstn epoch model delays, rem=%g sec, ref=%gsec", 0, rem_delay, ref_delay);
    // msg ("refstn epoch model rates, rem=%g sec/sec, ref=%g sec/sec", 0, rem_rate, ref_rate);
    // *delay_ref = rem_delay - ref_delay;
    // *rate_ref  = (rem_rate - ref_rate) * ref_doppler;
    // *ref_stn_delay = ref_delay;
    // 
    
}

}
