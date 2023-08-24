#include "ffit.hh"

#include "MHO_ComputePlotData.hh"

mho_json construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, mho_json& vexInfo)
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
    
    std::string directory = paramStore->GetAs<std::string>("/cmdline/directory");
    std::string control_file = paramStore->GetAs<std::string>("/cmdline/control_file");
    std::string baseline = paramStore->GetAs<std::string>("/cmdline/baseline");
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");
    
    //TODO FIXME Organize all the plot data generation better
    int c_mbdmax = paramStore->GetAs<int>("/fringe/max_mbd_bin");
    int c_sbdmax = paramStore->GetAs<int>("/fringe/max_sbd_bin");
    int c_drmax = paramStore->GetAs<int>("/fringe/max_dr_bin");
    double sbdelay = paramStore->GetAs<double>("/fringe/sbdelay");
    double mbdelay = paramStore->GetAs<double>("/fringe/mbdelay");
    double drate = paramStore->GetAs<double>("/fringe/drate");
    double frate = paramStore->GetAs<double>("/fringe/frate");
    double famp = paramStore->GetAs<double>("/fringe/famp");

    std::string mbd_anchor;
    bool is_mbd_anchor_set = paramStore->Get(std::string("mbd_anchor"), mbd_anchor);

    MHO_ComputePlotData mk_plotdata;
    bool optimize_closure_flag = false;
    bool is_oc_set = paramStore->Get(std::string("optimize_closure"), optimize_closure_flag );
    if(optimize_closure_flag){mk_plotdata.EnableOptimizeClosure();} //this does have an effect on overall fringe phase
    if(is_mbd_anchor_set){mk_plotdata.SetMBDAnchor(mbd_anchor);} //effect not yet implemented


    double total_ap_frac = paramStore->GetAs<double>("/fringe/total_summed_weights");
    mk_plotdata.SetSummedWeights(total_ap_frac);
    mk_plotdata.SetReferenceFrequency(ref_freq);
    mk_plotdata.SetMBDelay(mbdelay);
    mk_plotdata.SetDelayRate(drate);
    mk_plotdata.SetFringeRate(frate);
    mk_plotdata.SetSBDelay(sbdelay);
    mk_plotdata.SetSBDelayBin(c_sbdmax);
    mk_plotdata.SetAmplitude(famp);

    mk_plotdata.SetSBDArray(sbd_data);
    mk_plotdata.SetWeights(wt_data);
    mk_plotdata.SetVisibilities(vis_data);

    mk_plotdata.SetVexInfo(vexInfo);

    mho_json plot_dict;
    mk_plotdata.DumpInfoToJSON(plot_dict);

    mho_json sched_section = vexInfo["$SCHED"];
    std::string scan_name = sched_section.begin().key();
    auto sched_info = sched_section.begin().value();
    plot_dict["RootScanBaseline"] = "place_holder";//scanStore.GetRootFileBasename() + ", " + scan_name + ", " + baseline;
    plot_dict["CorrVers"] = "HOPS4/DiFX fourfit  rev 0";
    plot_dict["PolStr"] = polprod;

    //DEBUG! open and dump to file
    std::string output_file = "fdump.json"; //for testing
    std::string fdump = output_file;
    std::ofstream fdumpFile(fdump.c_str(), std::ofstream::out);
    fdumpFile << plot_dict;
    fdumpFile.close();

    return plot_dict;
}
