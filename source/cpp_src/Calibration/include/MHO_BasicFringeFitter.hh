#ifndef MHO_BasicFringeFitter_HH__
#define MHO_BasicFringeFitter_HH__

#include "MHO_FringeFitter.hh"

/*
*File: MHO_BasicFringeFitter.hh
*Class: MHO_BasicFringeFitter
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: basic single-baseline fringe fitter, no bells or whistles
*/

namespace hops 
{

class MHO_BasicFringeFitter: public MHO_FringeFitter
{
        
    public:
        MHO_BasicFringeFitter();
        virtual ~MHO_BasicFringeFitter();
        
        //basic run scheme: configure, init, then while(!IsFinished() ){ pre-run, run, post-run }
        virtual void Configure() override;
        virtual void Initialize() override;
        virtual void PreRun() override;
        virtual void Run() override;
        virtual void PostRun() override;
        virtual bool IsFinished() override;

    protected:
        
        //helper functions -- TODO determine which (if any) of these should be made public
        static std::string leftpadzeros_integer(unsigned int n_places, int value);
        static std::string make_legacy_datetime_format(legacy_hops_date ldate);
        static double calculate_snr(double effective_npol, double ap_period, double samp_period, double total_ap_frac, double amp);
        static double calculate_mbd_no_ion_error(double freq_spread, double snr);
        static double calculate_sbd_error(double sbd_sep, double snr, double sbavg);
        static double calculate_drate_error_v1(double snr, double ref_freq, double total_nap, double ap_delta);
        static double calculate_drate_error_v2(double snr, double ref_freq, double integration_time);
        static double calculate_pfd(double snr, double pts_searched);
        static double calculate_phase_error(double sbavg, double snr);
        static double calculate_phase_delay_error(double sbavg, double snr, double ref_freq);
        static std::string calculate_qf();
        static double calculate_residual_phase(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
        static void correct_phases_mbd_anchor_sbd(double ref_freq, double freq0, double frequency_spacing, double delta_mbd, double& totphase_deg, double& resphase_deg)



        
        
        //more helper functions
        static void configure_data_library(MHO_ContainerStore* store);
        static void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        static void calculate_fringe_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo);
        static void fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict);
        static void precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        static void init_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category);
        static int parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore);
        static void basic_fringe_search(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        static mho_json construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, mho_json& vexInfo);
        static void set_default_parameters(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);

};

}//end namespace

#endif /* end of include guard: MHO_BasicFringeFitter_HH__ */
