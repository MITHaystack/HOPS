#include "MHO_FringePlotInfo.hh"

//construct_plot_data
#include "MHO_ComputePlotData.hh"

#include "MHO_BasicFringeInfo.hh"

namespace hops
{

mho_json
MHO_FringePlotInfo::construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, mho_json& vexInfo)
{
    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    visibility_type* sbd_data = conStore->GetObject<visibility_type>(std::string("sbd"));
    //visibility_type* sbd_dr_data = conStore->GetObject<weight_type>(std::string("sbd_dr"));

    //test grab the reference freq
    double ref_freq = paramStore->GetAs<double>(std::string("/config/ref_freq"));

    std::string directory = paramStore->GetAs<std::string>("/files/directory");
    std::string control_file = paramStore->GetAs<std::string>("/files/control_file");
    std::string baseline = paramStore->GetAs<std::string>("/config/baseline");
    std::string polprod = paramStore->GetAs<std::string>("/config/polprod");
    std::string root_file = paramStore->GetAs<std::string>("/files/root_file");

    std::string mbd_anchor;
    bool is_mbd_anchor_set = paramStore->Get(std::string("mbd_anchor"), mbd_anchor);

    MHO_ComputePlotData mk_plotdata;
    mk_plotdata.SetParameterStore(paramStore);
    mk_plotdata.SetContainerStore(conStore);
    mk_plotdata.SetVexInfo(vexInfo);
    bool optimize_closure_flag = false;
    bool is_oc_set = paramStore->Get(std::string("optimize_closure"), optimize_closure_flag );
    if(optimize_closure_flag){mk_plotdata.EnableOptimizeClosure();} //this does have an effect on overall fringe phase
    if(is_mbd_anchor_set){mk_plotdata.SetMBDAnchor(mbd_anchor);}
    mk_plotdata.Initialize();

    mho_json plot_dict;
    mk_plotdata.DumpInfoToJSON(plot_dict);

    mho_json sched_section = vexInfo["$SCHED"];
    std::string scan_name = sched_section.begin().key();
    auto sched_info = sched_section.begin().value();
    plot_dict["RootScanBaseline"] = root_file + ", " + scan_name + ", " + baseline;
    plot_dict["CorrVers"] = "HOPS4/DiFX fourfit rev 4.0";

    auto ref_name = paramStore->GetAs<std::string>("/ref_station/site_name");
    auto rem_name = paramStore->GetAs<std::string>("/rem_station/site_name");
    std::string freq_group = "fgroup ?";

    plot_dict["PolStr"] = ref_name + " - " + rem_name +", " + freq_group + ", " + "pol " + polprod;

    //DEBUG! open and dump to file
    std::string output_file = "fdump.json"; //for testing
    std::string fdump = output_file;
    std::ofstream fdumpFile(fdump.c_str(), std::ofstream::out);
    fdumpFile << plot_dict;
    fdumpFile.close();

    return plot_dict;
}


void
MHO_FringePlotInfo::fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict)
{
    plot_dict["Quality"] = paramStore->GetAs<std::string>("/fringe/quality_code");
    plot_dict["SNR"] = paramStore->GetAs<double>("/fringe/snr");
    plot_dict["IntgTime"] = paramStore->GetAs<double>("/fringe/integration_time");
    plot_dict["Amp"] = paramStore->GetAs<double>("/fringe/famp");
    plot_dict["ResPhase"] = paramStore->GetAs<double>("/fringe/resid_phase");
    plot_dict["PFD"] = paramStore->GetAs<double>("/fringe/prob_false_detect");

    plot_dict["ResidSbd(us)"] = paramStore->GetAs<double>("/fringe/sbdelay");
    plot_dict["ResidMbd(us)"] = paramStore->GetAs<double>("/fringe/mbdelay");
    plot_dict["FringeRate(Hz)"]  = paramStore->GetAs<double>("/fringe/frate");
    plot_dict["IonTEC(TEC)"] = "-";
    plot_dict["RefFreq(MHz)"] = paramStore->GetAs<double>("/config/ref_freq");
    plot_dict["AP(sec)"] = paramStore->GetAs<double>("/config/ap_period");
    plot_dict["ExperName"] = paramStore->GetAs<std::string>("/vex/experiment_name");
    plot_dict["ExperNum"] = paramStore->GetAs<std::string>("/vex/experiment_number");

    //export the legacy date/time stamps for start, stop, and FRT
    plot_dict["YearDOY"] = paramStore->GetAs<std::string>("/fringe/year_doy");
    plot_dict["Start"] = paramStore->GetAs<std::string>("/fringe/legacy_start_timestamp");
    plot_dict["Stop"] = paramStore->GetAs<std::string>("/fringe/legacy_stop_timestamp");
    plot_dict["FRT"] = paramStore->GetAs<std::string>("/fringe/legacy_frt_timestamp");
    plot_dict["CorrTime"] = "-";
    plot_dict["FFTime"] = "-";
    plot_dict["BuildTime"] = "-";

    plot_dict["RA"] = paramStore->GetAs<std::string>("/vex/scan/source/ra");
    plot_dict["Dec"] = paramStore->GetAs<std::string>("/vex/scan/source/dec");

    //in order to follow the PDD interface, the name of the following output
    //parameter changes depending on whether or not we are using mbd_anchor = sbd

    std::string mbd_anchor = paramStore->GetAs<std::string>("mbd_anchor");
    if(mbd_anchor == "sbd")
    {
        plot_dict["GroupDelaySBD(usec)"] = paramStore->GetAs<double>("/fringe/total_mbdelay");
    }
    else
    {
        plot_dict["GroupDelayModel(usec)"] = paramStore->GetAs<double>("/fringe/total_mbdelay");
    }
    plot_dict["SbandDelay(usec)"] = paramStore->GetAs<double>("/fringe/total_sbdelay");
    plot_dict["DelayRate(ps/s)"] = paramStore->GetAs<double>("/fringe/total_drate");
    plot_dict["PhaseDelay(usec)"] = paramStore->GetAs<double>("/fringe/phase_delay");
    plot_dict["TotalPhase(deg)"] = paramStore->GetAs<double>("/fringe/tot_phase");

    plot_dict["AprioriClock(usec)"] = paramStore->GetAs<double>("/fringe/relative_clock_offset");
    plot_dict["AprioriClockrate(us/s)"] = paramStore->GetAs<double>("/fringe/relative_clock_rate");
    plot_dict["AprioriDelay(usec)"] = paramStore->GetAs<double>("/model/adelay");
    plot_dict["AprioriRate(us/s)"] = paramStore->GetAs<double>("/model/arate");
    plot_dict["AprioriAccel(us/s/s)"] = paramStore->GetAs<double>("/model/aaccel");

    plot_dict["ResidMbdelay(usec)"] = paramStore->GetAs<double>("/fringe/mbdelay");
    plot_dict["ResidSbdelay(usec)"] = paramStore->GetAs<double>("/fringe/sbdelay");
    plot_dict["ResidPhdelay(usec)"] = paramStore->GetAs<double>("/fringe/resid_ph_delay");
    plot_dict["ResidRate(us/s)"] = paramStore->GetAs<double>("/fringe/drate");
    plot_dict["ResidPhase(deg)"] = paramStore->GetAs<double>("/fringe/resid_phase"); //degrees

    plot_dict["ResidMbdelayError(usec)"] = paramStore->GetAs<double>("/fringe/mbd_error");
    plot_dict["ResidSbdelayError(usec)"] = paramStore->GetAs<double>("/fringe/sbd_error");
    plot_dict["ResidPhdelayError(usec)"] = paramStore->GetAs<double>("/fringe/phase_delay_error");
    plot_dict["ResidRateError(us/s)"] = paramStore->GetAs<double>("/fringe/drate_error");
    plot_dict["ResidPhaseError(deg)"] =  paramStore->GetAs<double>("/fringe/phase_error");


    //the quantities below are stuff that is on the fringe-plot but is not present
    //in the plot-data-dir file structure, so we put them under 'extra'
    
    plot_dict["extra"]["ambiguity"] =  paramStore->GetAs<double>("/fringe/ambiguity");

    plot_dict["extra"]["ref_station"]["az"] = paramStore->GetAs<double>("/ref_station/azimuth");
    plot_dict["extra"]["ref_station"]["al"] = paramStore->GetAs<double>("/ref_station/elevation");
    plot_dict["extra"]["ref_station"]["pa"] = paramStore->GetAs<double>("/ref_station/parallactic_angle");

    double refu = paramStore->GetAs<double>("/ref_station/u");
    double refv = paramStore->GetAs<double>("/ref_station/v");

    plot_dict["extra"]["ref_station"]["u"] = refu;
    plot_dict["extra"]["ref_station"]["v"] = refv;
    plot_dict["extra"]["ref_station"]["w"] = paramStore->GetAs<double>("/ref_station/w");
    plot_dict["extra"]["rem_station"]["az"] = paramStore->GetAs<double>("/rem_station/azimuth");
    plot_dict["extra"]["rem_station"]["al"] = paramStore->GetAs<double>("/rem_station/elevation");
    plot_dict["extra"]["rem_station"]["pa"] = paramStore->GetAs<double>("/rem_station/parallactic_angle");

    double remu = paramStore->GetAs<double>("/rem_station/u");
    double remv = paramStore->GetAs<double>("/rem_station/v");
    plot_dict["extra"]["rem_station"]["u"] = remu;
    plot_dict["extra"]["rem_station"]["v"] = remv;
    plot_dict["extra"]["rem_station"]["w"] = paramStore->GetAs<double>("/rem_station/w");

    plot_dict["extra"]["control_file"] = paramStore->GetAs<std::string>("/files/control_file");
    plot_dict["extra"]["baseline_input_file"] = paramStore->GetAs<std::string>("/files/baseline_input_file");
    plot_dict["extra"]["ref_station_input_file"] = paramStore->GetAs<std::string>("/files/ref_station_input_file");
    plot_dict["extra"]["rem_station_input_file"] = paramStore->GetAs<std::string>("/files/rem_station_input_file");
    plot_dict["extra"]["output_file"] = paramStore->GetAs<std::string>("/files/output_file");

    plot_dict["extra"]["ref_station_mk4id"] = paramStore->GetAs<std::string>("/ref_station/mk4id");
    plot_dict["extra"]["rem_station_mk4id"] = paramStore->GetAs<std::string>("/rem_station/mk4id");
    
    plot_dict["extra"]["ref_station_sample_bits"] = paramStore->GetAs<int>("/ref_station/sample_bits");
    plot_dict["extra"]["rem_station_sample_bits"] = paramStore->GetAs<int>("/rem_station/sample_bits");

    //calculate the (u,v) coordinates (taken from fill_202.c)
    double speed_of_light_Mm = 299.792458; // in mega-meters (?!)
    double lambda = speed_of_light_Mm / paramStore->GetAs<double>("/config/ref_freq"); // wavelength (m)
    double radians_to_arcsec = 4.848137e-6;
    double du = radians_to_arcsec*(remu - refu) /lambda;
    double dv = radians_to_arcsec*(remv - refv) /lambda;
    plot_dict["extra"]["u"] = du;
    plot_dict["extra"]["v"] = dv;
    
    //put the sample rate here
    double srate = paramStore->GetAs<double>("/vex/scan/sample_rate/value");
    double srate_MHz = srate/1e6; //convert to MHz
    plot_dict["extra"]["sample_rate"] = srate_MHz;
}




}//end namespace
