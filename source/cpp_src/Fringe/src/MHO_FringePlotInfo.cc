#include "MHO_FringePlotInfo.hh"

//construct_plot_data
#include "MHO_ComputePlotData.hh"

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
    double ref_freq = paramStore->GetAs<double>(std::string("ref_freq"));
    
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
    plot_dict["CorrVers"] = "HOPS4/DiFX fourfit  rev 0";
    
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
    plot_dict["RefFreq(MHz)"] = paramStore->GetAs<double>("ref_freq");
    plot_dict["AP(sec)"] = paramStore->GetAs<double>("ap_period");
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
}




}//end namespace
